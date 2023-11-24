
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "account.h"

#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024

enum {
    user_register = 1,
    sign_in,
    search,
    sign_out
};

enum {
    false,
    true
};

enum {
    blocked,
    active
};

//input file
char c_fileName[256] = "account.txt";

account *head = NULL;
account *current = NULL;

//insert a link to head of linked list
void InsertFirst(account acc) {
    account *link = (account *) malloc(sizeof(account));
    strcpy(link->m_username, acc.m_username);
    strcpy(link->m_password, acc.m_password);
    link->m_status = acc.m_status;
    link->next = head;
    head = link;
}

//read data from file and add to linked list
void readData() {
    account acc;
    FILE *f;
    //r == read file text
    if (!(f = fopen(c_fileName, "r"))) {
        printf("\n File not found!! \n\n");
    } else {
        while (!feof(f)) {
            fscanf(f, "%s %s %d", acc.m_username, acc.m_password, &acc.m_status);
            acc.m_isLogin = 0;
            acc.m_numOfFalseLogin = 0;
            InsertFirst(acc);
        }
    }
    fclose(f);
}

//rewrite data after any change.
void rewriteData() {
    FILE *f;
    if (!(f = fopen(c_fileName, "w"))) {
        printf("\n File not found!! \n\n");
    } else {
        account *ptr;
        ptr = head;
        while (ptr != NULL) {
            fprintf(f, "%s %s %d\n", ptr->m_username, ptr->m_password, ptr->m_status);
            ptr = ptr->next;
        }
    }
    fclose(f);
}

void handleLogin(int conn_sock, char username[], char password[]) {
    int isDone = false;

    do {
        account *ptr;
        ptr = head;
        int isExist = false;
        while (ptr != NULL) {
            //check if username is exist or not
            if (strcmp(username, ptr->m_username) == 0) {
                //if username is found in list,
                isExist = true;
                //check user status
                if (ptr->m_status == blocked) {
                    char *msg = "This account is blocked\n ";
                    send(conn_sock, msg, strlen(msg), 0);
                    printf("Account \"%s\" is blocked.\n", ptr->m_username);
                }
                    /* else if (ptr->m_isLogin == true)
                    {
                        char *msg = "This account is allready login\n ";
                        send(conn_sock, msg, strlen(msg), 0);
                        printf("Account \"%s\" is allready login\n", ptr->m_username);
                    } */
                else {
                    //if this account is active, check password
                    if (strcmp(password, ptr->m_password) != 0) {
                        ptr->m_numOfFalseLogin++;
                        char *msg = "Password is incorrect.\n";
                        //after 3 time wrong pass, set status to "blocked"
                        if (ptr->m_numOfFalseLogin == 3) {
                            ptr->m_status = blocked;
                            isDone = true;
                            ptr->m_numOfFalseLogin = 0;
                            //change data in file txt (status)
                            rewriteData();
                            char firstloglocation[BUFF_SIZE];
                            strcpy(firstloglocation, msg);
                            msg = strcat(firstloglocation, "Password is incorrect 3 times. You are blocked\n");
                            send(conn_sock, msg, strlen(msg), 0);
                            printf("Password is incorrect 3 times. Account \"%s\" is blocked.\nContract administrator for more information.\n",
                                   ptr->m_username);
                        } else {
                            send(conn_sock, msg, strlen(msg), 0);
                            printf("%s", msg);
                        }
                    }
                        //login success
                    else {
                        isDone = true;
                        ptr->m_isLogin = true;
                        ptr->m_numOfFalseLogin = 0;
                        char *msg = "Hello! Successful login.\n";
                        //strcat(msg, ptr->m_username);
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
    //signal();
    if (argc != 2) {
        printf("error, too many or too few arguments\n");
        printf("Correct format is /.server YourPort\n");
        return 1;
    }
    readData();
    int listen_sock, conn_sock; /* file descriptors */

    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;

    //Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    //Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));     /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server)) == -1) { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    //Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1) { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    pthread_t tid;
    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *) &client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
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
        //receives username from client
        recv(conn_sock, username, BUFF_SIZE, 0);
        send(conn_sock, "Success username\n", sizeof("Success username\n"), 0);
        username[strlen(username) - 1] = '\0';
        printf("Username: %s\n", username);
        //receives password from client
        recv(conn_sock, password, BUFF_SIZE, 0);
        send(conn_sock, "Success password\n", sizeof("Success password\n"), 0);
        password[strlen(password) - 1] = '\0';
        printf("Password: %s\n", password);
        sleep(1);
        handleLogin(conn_sock, username, password);
        printf("-------------------------\n");
    }
    close(conn_sock);
}