#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 8192

#define SEND_MESSAGE       1
#define UPLOAD_FILE        2
#define EXIT               0
#define SELECT_IN_PROGRESS -1

void menu() {
    printf("\nMENU\n");
    printf("-----------------------------------\n");
    printf("1. Gửi xâu bất kỳ \n");
    printf("2. Gửi nội dung một file\n");
    printf("0. Exit\n\n");
    printf("Select: ");
}

int main(int argc, char *argv[]) {
    int bytes_received;
    FILE *f;
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    char *SERV_IP = argv[1];

    char *endptr;
    errno = 0;
    uint16_t SERV_PORT = strtoul(argv[2], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        printf("Error: %s\n", errno == EINVAL ? "invalid base" : "invalid input");
        return 0;
    }

    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int bytes_sent;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        printf("\nError!Can not connect to sever! Client exit immediately! ");
        return 0;
    }

    // Step 4: Communicate with server

    int client_status = SELECT_IN_PROGRESS;
    int choice = -1;

    while (true) {
        memset(buff, '\0', BUFF_SIZE);
        // send message
        if (client_status == SELECT_IN_PROGRESS) {
            while (true) {
                menu();
                memset(buff, '\0', BUFF_SIZE);
                fgets(buff, BUFF_SIZE, stdin);
                buff[strlen(buff) - 1] = '\0';
                choice = strtol(buff, &endptr, 10);
                if (endptr == buff || *endptr != '\0') {
                    printf("Error: invalid input\n");
                    continue;
                } else if (choice != SEND_MESSAGE && choice != UPLOAD_FILE && choice != EXIT) {
                    printf("Error: invalid choice\n");
                    continue;
                } else {
                    break;
                }
            }
            if (choice == EXIT) {
                strcpy(buff, "0\n");
                client_status = EXIT;
            } else if (choice == SEND_MESSAGE) {
                strcpy(buff, "1\n");
                client_status = SEND_MESSAGE;
            } else if (choice == UPLOAD_FILE) {
                strcpy(buff, "2\n");
                client_status = UPLOAD_FILE;
            }
            bytes_sent = send(client_sock, buff, strlen(buff), 0);
            if (bytes_sent <= 0) {
                printf("Error in sending data");
                break;
            }
        } else if (client_status == SEND_MESSAGE) {
            printf("\n> ");
            fgets(buff, BUFF_SIZE, stdin);
            if (strcmp(buff, "\n") == 0) {   // finish sending, return to main menu
                client_status = SELECT_IN_PROGRESS;
                choice = -1;
                bytes_sent = send(client_sock, "TERMINATE\n", strlen("TERMINATE\n"), 0);
                goto END;
            }
            bytes_sent = send(client_sock, buff, strlen(buff), 0);
            if (bytes_sent <= 0) {
                printf("Error in sending data");
                break;
            }
            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
            if (bytes_received <= 0) {
                printf("Error in receiving data");
                break;
            }
            buff[bytes_received] = '\0';
            printf("Reply from server: %s\n", buff);
        END:

        } else if (client_status == UPLOAD_FILE) {
            // send message
            printf("Enter filename: ");
            char file_name[50];
            fgets(file_name, 50, stdin);
            file_name[strlen(file_name) - 1] = '\0';

            if (file_name == NULL || strcmp(file_name, "\n") == 0) {
                client_status = SELECT_IN_PROGRESS;
                choice = -1;
                break;
            }
            if ((f = fopen(file_name, "r")) == NULL) {
                printf("Error: File not found\n");
                choice = -1;
                continue;
            }
            // send filename
            bytes_sent = send(client_sock, file_name, strlen(file_name), 0);
            if (bytes_sent <= 0) {
                printf("\nConnection closed!\n");
                break;
            }
            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
            if (bytes_received <= 0) {
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }
            buff[bytes_received] = '\0';
            if (strcmp(buff, "File existed") == 0) {
                printf("%s!\n", buff);
                client_status = SELECT_IN_PROGRESS;
                continue;
            }

            int nread = 0;
            // read content of the file and send to server
            while ((nread = fread(buff, sizeof(char), BUFF_SIZE, f)) > 0) {
                bytes_sent = send(client_sock, buff, nread, 0);
                if (bytes_sent <= 0) {
                    printf("Error in sending data");
                    break;
                }
            }
            sleep(1);   // make sure EOF signal does not be concatenated to the file content.
            // EOF
            strcpy(buff, "END");
            bytes_sent = send(client_sock, buff, sizeof(buff), 0);
            if (bytes_sent <= 0) {
                printf("Error in sending data");
                break;
            }
            fclose(f);
            client_status = SELECT_IN_PROGRESS;
            choice = -1;
        } else {   // client wants to exit
            choice = -1;
            break;
        }
        fflush(stdout);
    }
    // Step 4: Close socket
    close(client_sock);
    return 0;
}
