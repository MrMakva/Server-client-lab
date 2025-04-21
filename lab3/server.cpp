#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>
using namespace std;

mutex mtx;

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[1024] = {0};
    while (true) {
        int valread = read(client_socket, buffer, 1024);
        if (valread <= 0) break;
        string message(buffer, valread);

        mtx.lock();
        ofstream outfile("output.txt", ios::app);
        outfile << message << endl;
        outfile.close();
        cout << "Received: " << message << endl;
        mtx.unlock();
    }
    close(client_socket);
    return nullptr;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(0); // Let the system choose a free port

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    getsockname(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    cout << "Server is running on port: " << ntohs(address.sin_port) << endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, handle_client, &new_socket);
        pthread_detach(thread_id);
    }

    return 0;
}
