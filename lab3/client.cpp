#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

void send_data(const string &ip, int port, int i) {
  int sock = 0;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "Socket creation error" << endl;
    return;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
    cerr << "Invalid address/ Address not supported" << endl;
    return;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    cerr << "Connection Failed" << endl;
    return;
  }

  for (int j = 0; j < 5; ++j) {
    string message =
        "Message " + to_string(j) + " from client " + to_string(i);
    send(sock, message.c_str(), message.length(), 0);
    this_thread::sleep_for(chrono::seconds(i));
  }

  close(sock);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <IP> <PORT>" << endl;
    return 1;
  }

  string ip = argv[1];
  int port = stoi(argv[2]);

  vector<thread> threads;
  for (int i = 1; i <= 3; ++i) {
    threads.emplace_back(send_data, ip, port, i);
  }

  for (auto &t : threads) {
    t.join();
  }

  return 0;
}