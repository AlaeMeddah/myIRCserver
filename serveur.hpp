#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <map>
#include <cerrno>
#include <cctype>

class Client
{
	private:
	public:
		int fd;
		std::string buffer;
		std::string username = "";

		Client() {}
		Client(int fd) {
			this->fd = fd;
		};
};

class Chatroom
{
	private:
	public:
	std::string name;
	std::vector<Client*> roomClients;
	Chatroom() {};
	Chatroom(Client* client, std::string name)
	{
		this->name = name;
		roomClients.push_back(client);
	}
};

class Server
{
	private:
		int 						socket_;
		std::string					port;
		pollfd 						server_fd;
		std::vector<struct pollfd>	pollfds;
		std::map<std::string, Chatroom>		chatrooms;
		std::map<int, Client> 		clients;

	public:
		Server(std::string port) {
			this->port = port;
		};
		void run();
		void handle_events();
		void server_socket();
		void on_client_connect();
		void send_message(int client_fd, const std::string &msg);
		bool receive_message(size_t index);
		void remove_client(size_t index);
		void process_client_buffer(Client &client);
		void process_message(Client &client, std::string message);
		void process_command(Client &client, std::string command);
		void broadcast(Client client, std::string message);
		void broadcast(std::string message);
		int	usernameExists(const std::string name);
		void setusername(Client &client, std::string command);
		void list(Client client);
		void whisper(Client client, std::string message);
		void room_creation(Client client, std::string room_name);
		int roomnameExist(const std::string name);
		std::string first_word(std::string &string);
};
/*
next step:
	/list
	/whisper
	chatrooms
	files and images
*/