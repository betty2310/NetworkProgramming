#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 8192

int isValidIpAddress(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if (result != 0)
        return 1;
    else
        return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("error, too many or too few arguments\n");
        printf("Correct format is /.client YourId YourPort\n");
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
        send(client_sock, username, strlen(username), 0);
        bytes_received = recv(client_sock, usernameMsg, BUFF_SIZE - 1, 0);
        usernameMsg[bytes_received] = '\0';

        // send password to server
        printf("Password: ");
        fgets(password, BUFF_SIZE, stdin);
        send(client_sock, password, strlen(password), 0);
        bytes_received = recv(client_sock, passwordMsg, BUFF_SIZE - 1, 0);
        passwordMsg[bytes_received] = '\0';

        // receive login status
        systemMsg_bytes_received = recv(client_sock, systemMsg, BUFF_SIZE - 1, 0);
        if (systemMsg_bytes_received <= 0) {
            printf("\nError!Cannot receive data from sever!\n");
            break;
        }
        systemMsg[systemMsg_bytes_received] = '\0';
        printf("%s\n", systemMsg);
        if (strcmp(systemMsg, "Hello! Successful login.\n") == 0) {
            printf("Press any key to continue\n");
            while (getchar() != '\n')
                ;
            continue;
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}