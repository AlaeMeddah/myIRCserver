#include "serveur.hpp"

void Server::server_socket()
{
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1)
        throw std::runtime_error("Error creating server socket");

    // Reuse address
    int enable = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
        throw std::runtime_error("Error setting SO_REUSEADDR");

    // Set socket to non-blocking
    int flags = fcntl(socket_, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Error getting socket flags");

    if (fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Error setting socket to non-blocking");

    // Prepare server address
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int port_num = std::stoi(port);
    if (port_num <= 0 || port_num > 65535)
        throw std::invalid_argument("Invalid port");

    server_address.sin_port = htons(static_cast<uint16_t>(port_num));

    // Bind
    if (bind(socket_,
             reinterpret_cast<sockaddr*>(&server_address),
             sizeof(server_address)) == -1)
        throw std::runtime_error("Error binding server socket");

    // Listen
    if (listen(socket_, SOMAXCONN) == -1)
        throw std::runtime_error("Error listening on server socket");
}

void Server::on_client_connect()
{
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(
            socket_,
            reinterpret_cast<sockaddr*>(&client_addr),
            &client_len
        );

        if (client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // No more clients waiting
            throw std::runtime_error(strerror(errno));
        }

        // Make client socket non-blocking
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        // Add client socket to poll
        pollfd pfd;
        pfd.fd = client_fd;
        pfd.events = POLLIN | POLLHUP;
        pfd.revents = 0;
        pollfds.push_back(pfd);

        clients.insert(std::make_pair(client_fd, Client(client_fd)));

        std::cout << "New client connected (fd="
                  << client_fd << ")\n";
    }
}

void Server::run()
{
    pollfd server_fd;
    server_fd.fd = socket_;
    server_fd.events = POLLIN;
    server_fd.revents = 0;

    pollfds.push_back(server_fd);

    while (true)
    {
        int ready = poll(pollfds.data(), pollfds.size(), -1);
        if (ready == -1)
            throw std::runtime_error("poll failed");
        handle_events();
    }
}

void Server::handle_events()
{
    for (size_t i = 0; i < pollfds.size(); ++i)
    {
        if (pollfds[i].fd == socket_ &&
            (pollfds[i].revents & POLLIN))
        {
            on_client_connect();
            continue;
        }

        if (pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        {
            remove_client(i);
            --i;
            continue;
        }

        if (pollfds[i].revents & POLLIN)
        {
            if (!receive_message(i))
            {
                remove_client(i);
                --i;
            }
        }

        pollfds[i].revents = 0;
    }
}

void Server::send_message(int client_fd, const std::string &msg)
{
    send(client_fd, msg.c_str(), msg.size(), 0);
}

bool Server::receive_message(size_t index)
{
    char buffer[1024];
    int fd = pollfds[index].fd;

    int bytes = recv(fd, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
        return false;

    // Append received data to the client's buffer
    clients[fd].buffer.append(buffer, bytes);

    // Process complete messages in the buffer
    process_client_buffer(clients[fd]);

    return true;
}

void Server::process_client_buffer(Client &client)
{
    std::string &buf = client.buffer;
    size_t pos;

    while ((pos = buf.find("\n")) != std::string::npos)
    {
        std::string message = buf.substr(0, pos);

        // Remove processed message + newline from buffer
        buf.erase(0, pos + 1);

        // Now message is complete, handle it
        std::cout << client.username <<": " << message << std::endl;
        process_message(client, message);
    }
}

void Server::process_message(Client &client, std::string message)
{
    if (message.empty())
        return;
    else if (message[0] == '/')
        process_command(client, message);
    else if (client.username.empty())
        send_message(client.fd, "Server message: Please choose a username using the command /NICK to use the server.\r\n");
    else
        broadcast(client, message);
}

void Server::process_command(Client &client, std::string command)
{
    if (command.compare(0, 6, "/NICK ") == 0)
    {
        setusername(client, command.substr(6));
        return;
    }
    if (client.username.empty())
    {
        send_message(client.fd, "Server message: Please choose a username using the command /NICK to use the server.\r\n");
        return;
    }
    else if (command.compare(0, 9,"/WHISPER ") == 0)
        whisper(client, command.substr(9));
    else if (command == "/LIST")
        list(client);
    // else if (command == "/HELP")
    //     help(client);
    else if (command.compare(0, 9, "/NEWROOM ") == 0)
        room_creation(client, command.substr(9));
    // add, kick, quit, promote
    else
        send_message(client.fd, "Server message: Could not resolve unknown command. Use /HELP to see a list of available command.\r\n");
}

void Server::room_creation(Client client, std::string room_name)
{
    if (room_name.find(' ') != std::string::npos)
    {
        send_message(client.fd, "Server message: /NEWROOM only takes one argument. Please try again.\r\n");
        return ;
    }
    if (roomnameExist(room_name))
    {
        send_message(client.fd, "Server message: There is already a room named "+ room_name +". Please try another one.\r\n");
        return ;
    }
    chatrooms.insert(std::make_pair(room_name, Chatroom(&client, room_name)));
}

int Server::roomnameExist(const std::string name)
{
    for (const auto &pair : chatrooms)
    {
        if (pair.first == name)
            return 1;
    }
    return -1;
}

void Server::whisper(Client client, std::string message)
{
    std::string receiver = first_word(message);
    int receiver_fd = usernameExists(receiver);
    if (receiver_fd == -1)
        send_message(client.fd, "Server message: Could not whisper to " + receiver + " as he is not a user on the server.\r\n");
    else
        send_message(receiver_fd, client.username + "(whisper): " + message + "\r\n");
}

std::string Server::first_word(std::string &str)
{
    size_t pos = str.find(' ');
    std::string first_word;

    if (pos == std::string::npos)
    {
        first_word = str;
        str = "";
    }
    else
    {
        first_word = str.substr(0, pos);
        str = str.substr(pos + 1);
    }
    return (first_word);
}

void Server::list(Client client)
{
    for (const auto &pair : clients)
    {
        if (client.fd == pair.second.fd)
            send_message(client.fd, "Server message: " + client.username + "\r\n");
        else
            send_message(client.fd, "Server message: " + pair.second.username + "(you)\r\n");
    }
}

void Server::setusername(Client &client, std::string username)
{
    if (username.find(' ') != std::string::npos)
    {
        send_message(client.fd, "Server message: /NICK only takes one argument. Please try again.\r\n");
        return ;
    }
    if (usernameExists(username) < 0)
    {
        client.username = username;
        broadcast(client.username +" has joined the server!\r\n");
        send_message(client.fd, "Server message: Welcome " + client.username +"!\r\n");
    }
    else
        send_message(client.fd, "Server message: Username is already in use. Please try another one.\r\n");
}

int Server::usernameExists(const std::string name)
{
    for (const auto &pair : clients)
    {
        const Client &client = pair.second;
        if (client.username == name)
            return client.fd;
    }
    return -1;
}

void Server::broadcast(Client client, std::string message)
{
    for (const auto &pair : clients)
    {
        if (pair.second.fd != client.fd)
            send_message(pair.second.fd, client.username + ": " + message);
    }
}

void Server::broadcast(std::string message)
{
    for (const auto &pair : clients)
    {
        send_message(pair.second.fd, "Server message: " + message + "\r\n");
    }
}

void Server::remove_client(size_t index)
{
    broadcast(clients[pollfds[index].fd].username + " has left the server.\r\n");
    std::cout << "Client fd=" << pollfds[index].fd << " disconnected\n";
    close(pollfds[index].fd);
    pollfds.erase(pollfds.begin() + index);
}
