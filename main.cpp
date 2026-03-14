#include "serveur.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    try
    {
        // 1️⃣ Create server object WITH constructor
        Server server(argv[1]);

        server.server_socket();    // bind & listen
        std::cout << "Server listening on port " << argv[1] << "...\n";

        // 2️⃣ Enter poll loop
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
