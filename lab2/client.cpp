#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(std::stoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_address.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    for (int i = 1; i <= 100; ++i) {
        std::string message = "Message " + std::to_string(i);
        send(client_socket, message.c_str(), message.size(), 0);
        std::cout << "Sent: " << message << std::endl;
        sleep(i);
    }

    close(client_socket);
    return 0;
}
