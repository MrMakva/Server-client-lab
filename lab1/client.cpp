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
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int num_messages = 5;
    int delay;

    for (int i = 1; i <= num_messages; i++) {
        delay = i;

        sprintf(buffer, "%d", i);

        sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        cout << "Sent message to server: " << buffer << endl;

        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&servaddr, (socklen_t*)sizeof(servaddr));
        buffer[n] = '\0';
        cout << "Received response from server: " << buffer << endl;

        sleep(delay);
    }

    close(sockfd);
    return 0;
}
