
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h> /* These are the usual header files */
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "linkedlist.h"

#define BACKLOG   2 /* Number of allowed connections */
#define BUFF_SIZE 1024

account *head;
account *current;

pthread_mutex_t *mptr;

enum { false, true };

enum { blocked, active };

void handleLogin(int conn_sock, char username[], char password[]) {
    int isDone = false;
    do {
        account *ptr;
        ptr = head;
        int isExist = false;
        while (ptr != NULL) {
            if (strcmp(username, ptr->m_username) == 0) {
                isExist = true;
                if (ptr->m_status == blocked) {
                    char *msg = "This account is blocked\n ";
                    send(conn_sock, msg, strlen(msg), 0);
                    printf("Account \"%s\" is blocked.\n", ptr->m_username);
                    isDone = true;
                    break;
                } else {
                    // if this account is active, check password
                    if (strcmp(password, ptr->m_password) != 0) {
                        ptr->m_numOfFalseLogin++;
                        char *msg;
                        isDone = true;
                        // after 3 time wrong pass, set status to "blocked"
                        if (ptr->m_numOfFalseLogin == 3) {
                            pthread_mutex_lock(mptr);
                            ptr->m_status = blocked;
                            ptr->m_numOfFalseLogin = 0;
                            rewriteData("account.txt");
                            pthread_mutex_unlock(mptr);
                            char firstloglocation[BUFF_SIZE];
                            strcpy(firstloglocation, msg);
                            msg = strcat(firstloglocation,
                                         "Password is incorrect 3 times. You are blocked\n");
                            send(conn_sock, msg, strlen(msg), 0);
                            printf(
                                "Password is incorrect 3 times. Account \"%s\" is "
                                "blocked.\nContract administrator for more information.\n",
                                ptr->m_username);
                        } else {
                            msg = "Password is incorrect!";
                            send(conn_sock, msg, strlen(msg), 0);
                            printf("%s\n", msg);
                            break;
                        }
                    } else {
                        isDone = true;
                        if (ptr->m_isLogin == true) {
                            char *msg = "This account is allready login\n ";
                            send(conn_sock, msg, strlen(msg), 0);
                            printf("Account \"%s\" is allready login!\n", ptr->m_username);
                            break;
                        }
                        ptr->m_isLogin = true;
                        ptr->m_numOfFalseLogin = 0;
                        char *msg = "Hello! Successful login.\n";
                        send(conn_sock, msg, strlen(msg), 0);
                        printf("Account \"%s\" login success.\n", ptr->m_username);
                    }
                }
            }
            ptr = ptr->next;
        }
        if (isExist == false) {
            char *msg = "This Account not exist!\n";
            send(conn_sock, msg, strlen(msg), 0);
            printf("Account \"%s\" is not exist.\n", username);
            isDone = true;
        }
    } while (!isDone);
}

void *client_handler(void *arg);

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("error, too many or too few arguments\n");
        printf("Correct format is /.server YourPort\n");
        return 1;
    }
    readData("account.txt");
    int listen_sock, conn_sock; /* file descriptors */

    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1])); /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) == -1) { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1) { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    pthread_t tid;
    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n",
               inet_ntoa(client.sin_addr)); /* prints client's IP */
        pthread_create(&tid, NULL, &client_handler, (void *) &conn_sock);
    }
    close(listen_sock);
    return 0;
}

void *client_handler(void *arg) {
    int conn_sock = *(int *) arg;
    pthread_detach(pthread_self());
    while (1) {
        char username[BUFF_SIZE];
        char password[BUFF_SIZE];
        memset(username, '\0', strlen(username));
        memset(password, '\0', strlen(password));
        printf("Receiving data ...\n");
        // receives username from client
        recv(conn_sock, username, BUFF_SIZE, 0);
        send(conn_sock, "Success username\n", sizeof("Success username\n"), 0);
        username[strlen(username) - 1] = '\0';
        printf("Username: %s\n", username);
        // receives password from client
        recv(conn_sock, password, BUFF_SIZE, 0);
        send(conn_sock, "Success password\n", sizeof("Success password\n"), 0);
        password[strlen(password) - 1] = '\0';
        printf("Password: %s\n", password);
        sleep(1);
        if (strcmp(username, "") != 0 && (strcmp(password, ""))) {
            handleLogin(conn_sock, username, password);
        }
        printf("-------------------------\n");
    }
    printf("close socker!");
    close(conn_sock);
}