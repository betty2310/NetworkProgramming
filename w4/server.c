#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 1024

void handle_message(char *buff, char *chars, char *nums);
void encode(char *message);
int valid_message(const char *message);

int main(int argc, char **argv) {
    int server_sock;
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server, client1, client2;
    int sin_size;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        perror("Error: ");
        return 1;
    }

    printf("Server run on port %s.\n\n", argv[1]);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Error: ");
        return 1;
    }

    memset(&buff, '\0', sizeof(buff));
    sin_size = sizeof(struct sockaddr_in);
    char chars[BUFF_SIZE];
    char nums[BUFF_SIZE];
    while (1) {
        bytes_received =
            recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client1, &sin_size);
        buff[bytes_received] = '\0';
        printf("[%s:%d]: %s\n", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
        sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *) &client2,
               sizeof(struct sockaddr));
        int bytes_received2 =
            recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client2, &sin_size);
        buff[bytes_received] = '\0';
        printf("[%s:%d]: %s\n", inet_ntoa(client2.sin_addr), ntohs(client2.sin_port), buff);
        sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *) &client1,
               sizeof(struct sockaddr));
    }

    close(server_sock);
    return 0;
}

void encode(char *message) {
    char key[] = "231002";
    for (int i = 0; i < strlen(message); i++) {
        message[i] ^= key[i % strlen(key)];
    }
}

int valid_message(const char *message) {
    for (int i = 0; message[i] != '\0'; i++) {
        if (!isalnum(message[i])) {
            return 0;
        }
    }
    return 1;
}

void handle_message(char *buff, char *chars, char *nums) {
    int char_index = 0;
    int num_index = 0;

    for (int i = 0; buff[i] != '\0'; i++) {
        if (isalpha(buff[i])) {
            chars[char_index++] = buff[i];
        } else if (isdigit(buff[i])) {
            nums[num_index++] = buff[i];
        }
    }

    chars[char_index] = '\0';
    nums[num_index] = '\0';
}
