#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int main(int argc, char** argv) {
    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received, sin_size;

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    char* SERV_IP = argv[1];
    int SERV_PORT = atoi(argv[2]);

    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("\nError: ");
        return 1;
    }

    printf("Client started.\n\n");

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    sin_size = sizeof(struct sockaddr);
    memset(buff, '\0', (strlen(buff) + 1));

    while (1) {
        memset(buff, '\0', (strlen(buff) + 1));

        printf("> ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strlen(buff) - 1] = '\0';
        if (strcmp(buff, "@") == 0 || strcmp(buff, "#") == 0) {
            printf("Quit!.\n");
            break;
        }
        bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr*) &server_addr,
                            sizeof(server_addr));
        bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0,
                                  (struct sockaddr*) &server_addr, &sin_size);
        buff[bytes_received] = '\0';
        printf("%s\n", buff);
    }

    close(client_sock);
    return 0;
}
