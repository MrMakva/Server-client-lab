#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_client(int client_socket) {
  char buffer[1024];
  while (true) {
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      break;
    }
    std::cout << "Received: " << buffer << std::endl;
  }
  close(client_socket);
}

void sigchld_handler(int signo) {
  while (waitpid(-1, nullptr, WNOHANG) > 0)
    ;
}

int main() {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    std::cerr << "Socket creation failed" << std::endl;
    return 1;
  }

  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = 0;

  if (bind(server_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    std::cerr << "Bind failed" << std::endl;
    close(server_socket);
    return 1;
  }

  socklen_t len = sizeof(server_address);
  getsockname(server_socket, (struct sockaddr *)&server_address, &len);
  std::cout << "Server is running on port: " << ntohs(server_address.sin_port)
            << std::endl;

  if (listen(server_socket, 5) < 0) {
    std::cerr << "Listen failed" << std::endl;
    close(server_socket);
    return 1;
  }

  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
    std::cerr << "Sigaction failed" << std::endl;
    close(server_socket);
    return 1;
  }

  while (true) {
    int client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket < 0) {
      std::cerr << "Accept failed" << std::endl;
      continue;
    }

    pid_t pid = fork();
    if (pid == 0) {
      close(server_socket);
      handle_client(client_socket);
      exit(0);
    } else if (pid > 0) {
      close(client_socket);
    } else {
      std::cerr << "Fork failed" << std::endl;
    }
  }

  close(server_socket);
  return 0;
}