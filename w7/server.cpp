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

#include <fstream>
#include <iostream>
#include <vector>

#define BUFF_SIZE 8192
#define BACKLOG   10

typedef struct {
    std::string username;
    std::string password;
    int status;
    int isLogin;
} Account;

std::vector<Account> accounts;

void readAccounts() {
    FILE *f;
    Account acc;
    if (!(f = fopen("accounts.txt", "r"))) {
        std::cout << "Cannot open file account.txt" << std::endl;
        return;
    } else {
        while (!feof(f)) {
            char username[100];
            char password[100];
            int status;
            int isLogin;
            fscanf(f, "%s %s %d %d", username, password, &status, &isLogin);
            acc.username = username;
            acc.password = password;
            acc.status = status;
            acc.isLogin = isLogin;
            accounts.push_back(acc);
        }
    }
    fclose(f);
}

void writeAccounts() {
    std::ofstream f("accounts.txt");
    if (!f) {
        std::cerr << "File could not be opened\n";
    } else {
        for (auto &acc : accounts) {
            f << acc.username << " " << acc.password << " " << acc.status << " " << acc.isLogin
              << "\n";
        }
    }
}

void handleLogin(int conn_sock, std::string username, std::string password) {
    readAccounts();
    std::cout << accounts.size() << std::endl;
    for (auto &acc : accounts) {
        if (acc.username == username && acc.password == password) {
            if (acc.status == 0) {
                send(conn_sock, "Account is blocked\n", sizeof("Account is blocked\n"), 0);
                return;
            } else if (acc.isLogin == true) {
                send(conn_sock, "Account is logged in\n", sizeof("Account is logged in\n"), 0);
                return;
            } else {
                acc.isLogin = true;
                send(conn_sock, "Login success\n", sizeof("Login success\n"), 0);
                writeAccounts();
                return;
            }
        }
    }
    send(conn_sock, "Wrong username or password\n", sizeof("Wrong username or password\n"), 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int listen_sock, conn_sock; /* file descriptors */
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in client;  /* client's address information */
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

    printf("Server started!\n");

    int pid = 0;

    // begin communicate
    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n",
               inet_ntoa(client.sin_addr)); /* prints client's IP */

        pid = fork();
        if (pid < 0) {
            close(conn_sock);
            continue;
        } else if (pid > 0) {   // parent
            close(conn_sock);
            continue;
        } else {   // child
            while (1) {
                char username[100];
                recv(conn_sock, username, 100, 0);
                send(conn_sock, "OK", sizeof("OK"), 0);
                username[strlen(username) - 1] = '\0';

                char password[100];
                recv(conn_sock, password, 100, 0);
                send(conn_sock, "OK\n", sizeof("OK\n"), 0);
                password[strlen(password) - 1] = '\0';

                std::string s_username = username;
                std::string s_password = password;
                sleep(1);
                handleLogin(conn_sock, s_username, s_password);
            }
        }
    }

    close(listen_sock);
    return 0;
}