#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
using namespace std;

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int find_free_port() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(0);

  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(sock);
    return -1;
  }

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(sock);
    return -1;
  }

  socklen_t len = sizeof(addr);
  if (getsockname(sock, (struct sockaddr *)&addr, &len) < 0) {
    perror("getsockname");
    close(sock);
    return -1;
  }

  int port = ntohs(addr.sin_port);
  close(sock);
  return port;
}

int main() {
  int master_socket, max_sd, activity, new_socket;
  struct sockaddr_in address;
  fd_set readfds;
  char buffer[BUFFER_SIZE];

  vector<int> client_sockets(MAX_CLIENTS, 0);

  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    perror("setsockopt");
    close(master_socket);
    exit(EXIT_FAILURE);
  }

  int port = find_free_port();
  if (port == -1) {
    close(master_socket);
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(master_socket);
    exit(EXIT_FAILURE);
  }

  cout << "Server started on port " << port << endl;

  if (listen(master_socket, 5) < 0) {
    perror("listen");
    close(master_socket);
    exit(EXIT_FAILURE);
  }

  int addrlen = sizeof(address);
  cout << "Waiting for connections..." << endl;

  while (true) {
    FD_ZERO(&readfds);

    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    for (int i = 0; i < MAX_CLIENTS; i++) {
      int sd = client_sockets[i];
      if (sd > 0) {
        FD_SET(sd, &readfds);
      }
      if (sd > max_sd) {
        max_sd = sd;
      }
    }

    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) {
      perror("select error");
      continue;
    }

    if (FD_ISSET(master_socket, &readfds)) {
      if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                               (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        continue;
      }

      cout << "New connection, socket fd: " << new_socket
                << ", ip: " << inet_ntoa(address.sin_addr)
                << ", port: " << ntohs(address.sin_port) << endl;

      bool added = false;
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
          client_sockets[i] = new_socket;
          cout << "Adding to list of sockets as " << i << endl;
          added = true;
          break;
        }
      }

      if (!added) {
        cout << "Too many connections, rejecting" << endl;
        const char *msg = "Server is busy, try again later\n";
        send(new_socket, msg, strlen(msg), 0);
        close(new_socket);
      }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
      int sd = client_sockets[i];
      if (sd > 0 && FD_ISSET(sd, &readfds)) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sd, buffer, BUFFER_SIZE - 1);

        if (valread == 0) {
          getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          cout << "Host disconnected, ip: " << inet_ntoa(address.sin_addr)
                    << ", port: " << ntohs(address.sin_port) << endl;
          close(sd);
          client_sockets[i] = 0;
        } else if (valread > 0) {
          buffer[valread] = '\0';
          if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
          }
          cout << "Received from client " << i << ": " << buffer
                    << endl;

          string ack = "Server received: " + string(buffer) + "\n";
          send(sd, ack.c_str(), ack.length(), 0);
        } else {
          if (errno == ECONNRESET) {
            cout << "Client " << i << " connection reset" << endl;
          } else {
            perror("read error");
          }
          close(sd);
          client_sockets[i] = 0;
        }
      }
    }
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (client_sockets[i] > 0) {
      close(client_sockets[i]);
    }
  }
  close(master_socket);

  return 0;
}