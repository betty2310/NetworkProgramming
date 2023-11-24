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

#define BUFF_SIZE   8192
#define MAXLINE     4096
#define EXISTEDFILE "Error: File da ton tai "
#define SEND_SIZE   1024

#define SEND_MESSAGE       1
#define UPLOAD_FILE        2
#define EXIT               0
#define SELECT_IN_PROGRESS -1

/* check if the given string is valid IPv4 address */
bool is_valid_ipv4_address(const char *str) {
    struct in_addr addr;
    return inet_pton(AF_INET, str, &addr) == 1;
}

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
    char sendline[MAXLINE];
    FILE *f;
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    // check if the IP address is valid
    char *SERV_IP = argv[1];
    if (!is_valid_ipv4_address(SERV_IP)) {
        printf("Error: invalid IP address\n");
        return 0;
    }
    // check if the port number is valid
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
            bzero(sendline, MAXLINE);
            fgets(sendline, MAXLINE, stdin);
            if (sendline == NULL || strcmp(sendline, "\n") == 0) {
                client_status = SELECT_IN_PROGRESS;
                choice = -1;
                break;
            }
            char fileName[MAXLINE];
            printf("fileName: %s\n", sendline);   // print file name
            if ((f = fopen(sendline, "rb")) == NULL) {
                printf("Error: File not found\n");
                fclose(f);
                continue;
            }
            strcpy(fileName, sendline);

            long filelen;
            fseek(f, 0, SEEK_END);   // Jump to the end of the file
            filelen = ftell(f);      // Get the current byte offset in the file
            rewind(f);               // pointer to start of file

            bytes_sent = send(client_sock, fileName, strlen(fileName), 0);   // send name of file
            if (bytes_sent <= 0) {                                           // check if false
                printf("\nConnection closed!\n");
                break;
            }

            bytes_received =
                recv(client_sock, buff, MAXLINE, 0);   // recv result of check file  name
            if (bytes_received <= 0) {
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }

            buff[bytes_received] = '\0';
            if (strcmp(buff, EXISTEDFILE) == 0) {   // if file is existed
                printf("%s\n", buff);
                continue;
            }

            bytes_sent = send(client_sock, &filelen, 20, 0);   // send file size
            if (bytes_sent <= 0) {
                printf("\nConnection closed!\n");
                break;
            }

            int sum = 0;   // count size byte send
            printf("\n Bat dau doc file !\n");
            while (1) {
                int byteNum = SEND_SIZE;
                if ((sum + SEND_SIZE) > filelen) {   // if over file size
                    byteNum = filelen - sum;
                }
                char *buffer = (char *) malloc((byteNum) * sizeof(char));
                fread(buffer, byteNum, 1, f);   // read buffer with size
                sum += byteNum;                 // increase byte send
                bytes_sent = send(client_sock, buffer, byteNum, 0);
                if (bytes_sent <= 0) {
                    printf("\nConnection closed!\n");
                    break;
                }
                free(buffer);
                if (sum >= filelen) {
                    break;
                }
            }
            printf("\n Ket thuc doc file !\n");
            fclose(f);   // close file

            bytes_received = recv(client_sock, buff, MAXLINE, 0);   // recv result
            if (bytes_received <= 0) {
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }

            printf("%s \n", buff);   // print result
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
