#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <server_ip> <server_port>"
              << endl;
    return 1;
  }

  const char *server_ip = argv[1];
  int port = stoi(argv[2]);

  int sock = 0;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "Socket creation error" << endl;
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    cerr << "Invalid address/ Address not supported" << endl;
    close(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    cerr << "Connection Failed" << endl;
    close(sock);
    return -1;
  }

  cout << "Connected to server. Enter a number between 1 and 10: ";
  int i;
  cin >> i;

  if (i < 1 || i > 10) {
    cerr << "Number must be between 1 and 10" << endl;
    close(sock);
    return -1;
  }

  cin.ignore(numeric_limits<streamsize>::max(), '\n');

  while (true) {
    string message = to_string(i);
    ssize_t sent_bytes = send(sock, message.c_str(), message.length(), 0);
    if (sent_bytes < 0) {
      perror("send failed");
      break;
    }
    cout << "Sent: " << message << endl;
    sleep(i);
  }

  close(sock);
  return 0;
}