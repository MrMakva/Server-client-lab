#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "Server is running and waiting for messages..." << endl;

    socklen_t len;
    int n;

    len = sizeof(cliaddr);

    while (true) {
        n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        cout << "Received message from client: " << buffer << endl;
        cout << "Client IP: " << inet_ntoa(cliaddr.sin_addr) << ", Port: " << ntohs(cliaddr.sin_port) << endl;

        int num = atoi(buffer);
        num++;
        sprintf(buffer, "%d", num);

        sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        cout << "Sent response to client: " << buffer << endl;
    }

    close(sockfd);
    return 0;
}
