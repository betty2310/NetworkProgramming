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

struct Account {
    char username[50];
    char password[50];
    int status;
};

void handle_message(char *buff, char *chars, char *nums);
void encode(char *message);
int valid_message(const char *message);

struct Account *accounts;

void init_accounts() {
    accounts = (struct Account *) malloc(2 * sizeof(struct Account));
    strcpy(accounts[0].username, "hust");
    strcpy(accounts[0].password, "hust123");
    accounts[0].status = 1;
    strcpy(accounts[1].username, "soict");
    strcpy(accounts[1].password, "soict123");
    accounts[1].status = 0;
}

int main(int argc, char **argv) {
    init_accounts();
    int server_sock;
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server, client;
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
    struct Account account;

    // handle username login
    char status[2];
handle_username:
    while (1) {
        int found = 0;
        bytes_received =
            recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client, &sin_size);
        buff[bytes_received] = '\0';
        printf("%s\n", buff);

        for (int i = 0; i < 2; i++) {
            if (strcmp(accounts[i].username, buff) == 0) {
                found = 1;
                account = accounts[i];
                strcpy(status, "1");
            }
        }
        if (!found) {
            strcpy(status, "0");
        }

        sendto(server_sock, status, strlen(status), 0, (struct sockaddr *) &client,
               sizeof(struct sockaddr));
        if (found) {
            goto handle_password;
        }
    }

handle_password:
    while (1) {
        bytes_received =
            recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client, &sin_size);
        buff[bytes_received] = '\0';
        printf("%s\n", buff);

        if (strcmp(account.password, buff) == 0) {
            if (account.status == 1) {
                strcpy(status, "1");
                sendto(server_sock, status, strlen(status), 0, (struct sockaddr *) &client,
                       sizeof(struct sockaddr));
                goto login_success;
            } else {
                strcpy(status, "2");
                sendto(server_sock, status, strlen(status), 0, (struct sockaddr *) &client,
                       sizeof(struct sockaddr));
                goto handle_password;
            }
        } else {
            strcpy(status, "0");
            sendto(server_sock, status, strlen(status), 0, (struct sockaddr *) &client,
                   sizeof(struct sockaddr));
            goto handle_username;
        }
    }
login_success:
    while (1) {
        bytes_received =
            recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client, &sin_size);
        buff[bytes_received] = '\0';

        if (!valid_message(buff)) {
            strcpy(buff, "Invalid message.");
            sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *) &client,
                   sizeof(struct sockaddr));
            continue;
        }
        char *chars = (char *) malloc(BUFF_SIZE * sizeof(char));
        char *nums = (char *) malloc(BUFF_SIZE * sizeof(char));
        memset(chars, '\0', BUFF_SIZE);
        memset(nums, '\0', BUFF_SIZE);
        handle_message(buff, chars, nums);

        // merge chars and nums
        strcpy(buff, chars);
        strcat(buff, "\t");
        strcat(buff, nums);

        printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
        sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *) &client,
               sizeof(struct sockaddr));
    }

    close(server_sock);
    free(accounts);
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
