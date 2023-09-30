#include "../headers/file_util.h"
#include "../headers/strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE* f;

node* read_file(char* file_name, node** head) {
    f = fopen(file_name, "r");
    if (f == NULL) {
        printf(FILE_NOT_FOUND, file_name);
        return NULL;
    }
    Account acc;
    acc.username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    acc.password = (char*) malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    int status;
    while(fscanf(f, "%s %s %d", acc.username, acc.password, &status) != EOF) {
	acc.status = (enum Status)status;
        push(head, acc);
        // reallocate memory for next account
        acc.username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
        acc.password = (char*) malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    }
    fclose(f);
    return *head;
}


void append_account_to_file(char* file_name, Account acc) {
    f = fopen(file_name, "a");
    fprintf(f, "\n%s %s %d", acc.username, acc.password, acc.status);
    fclose(f);
}

void change_account_status(char* file_name, Account* acc, enum Status status) {
    f = fopen(file_name, "r+");
    char* username = (char*) malloc(MAX_USERNAME_LENGTH * sizeof(char));
    char* password = (char*) malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    int status_int;
    while(fscanf(f, "%s %s %d", username, password, &status_int) != EOF) {
        if(!strcmp(username, acc->username)) {
            fseek(f, -1, SEEK_CUR);
            fprintf(f, "%d", status);
            break;
        }
    }
    fclose(f);
}
