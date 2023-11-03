#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "md5.h"

#define BACKLOG   2 /* Number of allowed connections */
#define BUFF_SIZE 8192
#define STORAGE   "./storage/"   // default save file location
#define SEND_SIZE 1024

#define SEND_MESSAGE       1
#define UPLOAD_FILE        2
#define EXIT               0
#define SELECT_IN_PROGRESS -1

int valid_message(const char *message) {
    for (int i = 0; message[i] != '\0'; i++) {
        if (!isalnum(message[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * Encodes a given string using MD5 algorithm.
 * @param buff The string to be encoded.
 * @return A pointer to the encoded string.
 */
char *encode(char *buff);

/**
 * @brief Processes a message by separating digits and letters.
 *
 * @param buff The message to be processed.
 * @param digits The buffer to store the digits found in the message.
 * @param letters The buffer to store the letters found in the message.
 * @return Returns 1 if the message was successfully processed, 0 otherwise.
 */
int handle_message(const char *buff, char *digits, char *letters);

int main(int argc, char *argv[]) {
    FILE *fileptr;
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int listen_sock, conn_sock;                 /* file descriptors */
    char recv_data[BUFF_SIZE];                  /* a buffer for sending and receiving data */
    char letters[BUFF_SIZE], digits[BUFF_SIZE]; /* saves the extracted strings */
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("\nError: ");
        return 0;
    }

    // Step 2: Bind address to socket
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));     /* the port to assign */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) ==
        -1) { /* associate with IP address and port number */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1) { /* listen for incoming connections */
        perror("\nError: ");
        return 0;
    }

    struct stat st = {0};

    if (stat(STORAGE, &st) == -1) {   // create storage if it not exist
        mkdir(STORAGE, 0755);
    }

    // Step 4: Communicate with client
    while (true) {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n",
               inet_ntoa(client.sin_addr)); /* prints client's IP */

        int client_status = SELECT_IN_PROGRESS;   // initialize the status of client

        // start conversation
        while (true) {
            memset(recv_data, '\0', BUFF_SIZE);

            // receives message from client
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
            if (bytes_received <= 0) {
                printf("Error in receiving data");
                break;
            }

            recv_data[bytes_received] = '\0';
            char *encoded = malloc(BUFF_SIZE);
            strcpy(encoded, encode(recv_data));

            if (client_status == SELECT_IN_PROGRESS) {
                recv_data[bytes_received - 1] = '\0';
                if (strcmp(recv_data, "1") == 0) {
                    client_status = SEND_MESSAGE;
                } else if (strcmp(recv_data, "2") == 0) {
                    client_status = UPLOAD_FILE;
                } else if (strcmp(recv_data, "0") == 0) {
                    client_status = EXIT;
                    strcpy(recv_data, "Goodbye! Chuc thay co 1 buoi toi vui ve!");
                    bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0);
                    if (bytes_sent <= 0) {
                        printf("Error in receiving data");
                    }
                    break;
                }
            } else if (client_status == SEND_MESSAGE) {
                memset(digits, '\0', BUFF_SIZE);
                memset(letters, '\0', BUFF_SIZE);
                recv_data[bytes_received - 1] = '\0';

                if (strcmp(recv_data, "") == 0 || strcmp(recv_data, "TERMINATE") == 0) {
                    client_status = SELECT_IN_PROGRESS;
                } else if ((handle_message(encoded, digits, letters) == 0) ||
                           (valid_message(recv_data) == 0)) {
                    strcpy(recv_data, "Invalid message");
                    bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0);
                    if (bytes_sent <= 0) {
                        printf("Error in sending data");
                        break;
                    }
                } else {
                    printf("Received message: %s\t%s\n", digits, letters);
                    bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0);
                    if (bytes_sent < 0) {
                        printf("Error in sending data");
                        break;
                    }
                }
            } else if (client_status == UPLOAD_FILE) {
                // handle filename
                if (strcmp(recv_data, "")) {
                    char name[100];
                    memset(name, '\0', 100);
                    strcpy(name, STORAGE);
                    strcat(name, recv_data);
                    printf("File name: %s\n\n", name);

                    bytes_sent = send(conn_sock, "OK", strlen("OK"), 0);
                    if (bytes_sent <= 0) {
                        printf("Error in sending data");
                        break;
                    }

                    fileptr = fopen(name, "wb");
                    while (1) {
                        bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
                        if (bytes_received <= 0) {
                            printf("Error in receiving data");
                            break;
                        }
                        if (strcmp(recv_data, "END") == 0) {
                            client_status = SELECT_IN_PROGRESS;
                            break;
                        }
                        recv_data[bytes_received] = '\0';
                        fwrite(recv_data, bytes_received, 1, fileptr);
                    }
                    printf("Uploaded file to ./storage/\n");
                    fclose(fileptr);
                } else {
                    client_status = SELECT_IN_PROGRESS;
                }
            }

            free(encoded);
            fflush(stdout);
        }
        // end connection
        close(conn_sock);
        fflush(stdout);
    }
    close(listen_sock);
    return 0;
}

char *encode(char *buff) {
    uint8_t result[16];
    md5String(buff, result);
    char *encoded = malloc(33);
    for (int i = 0; i < 16; ++i) {
        sprintf(encoded + i * 2, "%02x", result[i]);
    }
    encoded[32] = '\0';
    return encoded;
}

int handle_message(const char *buff, char *digits, char *letters) {
    int i = 0;
    int j = 0;
    int k = 0;
    while (buff[i] != '\0') {
        if (buff[i] >= '0' && buff[i] <= '9') {
            digits[j] = buff[i];
            ++j;
        } else if ((buff[i] >= 'a' && buff[i] <= 'z') || (buff[i] >= 'A' && buff[i] <= 'Z')) {
            letters[k] = buff[i];
            ++k;
        } else {
            return 0;
        }
        ++i;
    }
    digits[j] = '\0';
    letters[k] = '\0';
    return 1;
}