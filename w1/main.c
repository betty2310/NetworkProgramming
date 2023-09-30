#include "headers/file_util.h"
#include "headers/strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_TRY_LOGIN 3

FILE* f;
char* file_name;
node* head = NULL;
int is_login = 0;

Account* current_user;

Account* register_account();

Account* login();

void logout();

Account* search_account();

void run();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(HOW_TO_USE, argv[0]);
        return 1;
    }
    file_name = argv[1];
    head = read_file(file_name, &head);

    run();

    free_list(head);
    return 0;
}

void run() {
    int choice;
    do {
        if(is_login) {
            printf(LOGGED_MENU, current_user->username);
        } else {
            printf(MENU);
        }
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf(REGISTER);
                Account* new_account = register_account();
                if (new_account != NULL)
                    append_account_to_file(file_name, *new_account);
                break;
            case 2:
                printf(SIGN_IN);
                Account* logged_account = login();
                if (logged_account != NULL && logged_account->status == ACTIVE) {
                    is_login = 1;
                    current_user = logged_account;
                    printf(LOGGED_IN);
                    printf(HELLO_USER, current_user->username);
                }
                if(logged_account != NULL && logged_account->status == BLOCKED) {
                    change_account_status(file_name, logged_account, BLOCKED);
                }
                break;
            case 3:
                if(!is_login) {
                    printf(NOT_LOGGED_IN);
                    break;
                }
                search_account();
                break;
            case 4:
                logout();
                break;
            default:
                printf(GOOD_BYE);
                break;
        }
    } while (choice > 0 && choice < 5);
}

Account* register_account() {
    printf(USERNAME_PROMPT);
    char* username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    scanf("%s", username);
    Account *acc = (Account*) malloc(sizeof(Account));

    if (search(head, username)) {
        printf(EXISTED_ACCOUNT);
        return NULL;
    }
    printf(PASSWORD_PROMPT);
    char* password = (char*) malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    scanf("%s", password);
    acc->username = username;
    acc->password = password;
    acc->status = ACTIVE;
    push(&head, *acc);
    printf(REGISTER_SUCCESS);
    return acc;
}

Account* login() {
    printf(USERNAME_PROMPT);
    char* username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    scanf("%s", username);
    printf(PASSWORD_PROMPT);
    char* password = (char*) malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    scanf("%s", password);

    Account* acc = search(head, username);

    if(acc == NULL) {
        printf(ACCOUNT_NOT_FOUND);
        return NULL;
    }

    if(!strcmp(acc->password, password)) { // password is correct
        if(acc->status == BLOCKED) {
            printf(ACCOUNT_BLOCKED);
            return NULL;
        } else {
            return acc;
        }
    } else { // password is incorrect
        printf(INCORRECT_PASSWORD);
        int try = 0;
        while(try < NUM_TRY_LOGIN) {
            printf(TRY_AGAIN, NUM_TRY_LOGIN - try);
            scanf("%s", password);
            if(strcmp(acc->password, password) == 0) {
                if(acc->status == BLOCKED) {
                    printf(ACCOUNT_BLOCKED);
                    return NULL;
                }
                return acc;
            } else {
                try++;
            }
        }
        printf(ACCOUNT_BLOCKED);
        acc->status = BLOCKED;
    }
    return acc;
}

Account* search_account() {
    printf(USERNAME_PROMPT);
    char* username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    scanf("%s", username);

    Account* acc = search(head, username);

    if(acc == NULL) {
        printf(ACCOUNT_NOT_FOUND);
        return NULL;
    }

    printf("Account is %s\n", acc->status ? "active" : "blocked");
    return acc;
}

void logout() {
    if(!is_login) {
        printf(NOT_LOGGED_IN);
        return;
    }
    printf(USERNAME_PROMPT);
    char* username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    scanf("%s", username);

    if (strcmp(current_user->username, username) != 0) {
        printf("Wrong account name\n");
        return;
    }
    is_login = 0;
    printf(GOOD_BYE_USER, current_user->username);
}
