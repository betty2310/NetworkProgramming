#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "cypher.h"

#define BUFF_SIZE 8192

volatile quit = 0;

int key = 1234;

int isValidIpAddress(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if (result != 0)
        return 1;
    else
        return 0;
}
// Function that the thread will execute
void *receive_messages(void *arg) {
    int sockfd = *(int *) arg;   // Cast the argument to int
    char buffer[BUFF_SIZE];
    while (!quit) {
        memset(buffer, 0, sizeof(buffer));                  // Clear the buffer
        int n = read(sockfd, buffer, sizeof(buffer) - 1);   // Receive message from server
        if (n <= 0) {
            break;
        }
        if (strcmp(buffer, "quit\n") == 0) {
            quit = 1;
            break;
        } else {
            printf("%s\n", buffer);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./%s <adress> <port>\n", argv[0]);
        return 1;
    }
    // check if input id is valid
    if (isValidIpAddress(argv[1]) == 0) {
        printf("%s Not a valid ip address\n", argv[1]);
        return 1;
    }

    int client_sock;
    struct sockaddr_in server_addr; /* server's address information */

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        printf("\nError!Can not connect to sever! Client exit immediately! ");
        return 0;
    }

    // Step 4: Communicate with server
    while (1) {
        char username[BUFF_SIZE];
        char password[BUFF_SIZE];
        char usernameMsg[BUFF_SIZE];
        char passwordMsg[BUFF_SIZE];
        char systemMsg[BUFF_SIZE];
        int systemMsg_bytes_received;
        int bytes_received;
        memset(username, '\0', BUFF_SIZE);
        memset(password, '\0', BUFF_SIZE);
        printf("Username: ");
        fgets(username, BUFF_SIZE, stdin);
        char *encryptedUsername = encode(username, key);
        send(client_sock, encryptedUsername, strlen(encryptedUsername), 0);
        free(encryptedUsername);
        bytes_received = recv(client_sock, usernameMsg, BUFF_SIZE - 1, 0);
        usernameMsg[bytes_received] = '\0';

        // send password to server
        printf("Password: ");
        fgets(password, BUFF_SIZE, stdin);
        char *encryptedPassword = encode(password, key);
        send(client_sock, encryptedPassword, strlen(encryptedPassword), 0);
        free(encryptedPassword);
        bytes_received = recv(client_sock, passwordMsg, BUFF_SIZE - 1, 0);
        passwordMsg[bytes_received] = '\0';

        // receive login status
        systemMsg_bytes_received = recv(client_sock, systemMsg, BUFF_SIZE - 1, 0);
        if (systemMsg_bytes_received <= 0) {
            printf("\nError!Cannot receive data from sever!\n");
            break;
        }
        systemMsg[systemMsg_bytes_received] = '\0';
        username[strlen(username) - 1] = '\0';
        if (strcmp(systemMsg, "Hello! Successful login.\n") == 0) {
            printf("Hello %s, welcome to chat client\nSend 'quit' to logout\n", username);
            // create thread will always receive message from server
            pthread_t tid;
            pthread_create(&tid, NULL, &receive_messages, (void *) &client_sock);
            while (1) {
                char msg[BUFF_SIZE];
                printf("Menu:\n1. Send message\n2. Send image\n3. Quit\n");
                char choice;
                scanf("%c", &choice);
                getchar();
                if (choice == '1') {
                    fgets(msg, BUFF_SIZE, stdin);
                    char *encryptedMsg = encode(msg, key);
                    if (strcmp(msg, "quit\n") == 0) {
                        send(client_sock, msg, strlen(msg), 0);
                        quit = 1;
                        break;
                    } else {
                        send(client_sock, encryptedMsg, strlen(encryptedMsg), 0);
                    }
                    free(encryptedMsg);
                } else if (choice == '2') {
                    char *message = "send image\n";
                    send(client_sock, message, strlen(message), 0);
                    printf("Enter image path: ");
                    char imagePath[BUFF_SIZE];
                    fgets(imagePath, BUFF_SIZE, stdin);
                    imagePath[strlen(imagePath) - 1] = '\0';
                    FILE *image = fopen(imagePath, "rb");
                    if (image == NULL) {
                        printf("Cannot open image\n");
                        continue;
                    }
                    char *buffer = (char *) malloc(BUFF_SIZE);
                    int bytes_read;
                    while ((bytes_read = fread(buffer, sizeof(char), BUFF_SIZE, image)) > 0) {
                        send(client_sock, buffer, bytes_read, 0);
                    }
                    fclose(image);
                    free(buffer);
                    printf("Image \"%s\" sent to server\n", imagePath);
                } else if (choice == '3') {
                    char *quitMsg = "quit\n";
                    send(client_sock, quitMsg, strlen(quitMsg), 0);
                    quit = 1;
                    break;
                } else {
                    printf("Invalid choice\n");
                    continue;
                }
            }
            pthread_join(tid, NULL);
        } else {
            printf("%s\n", systemMsg);
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}