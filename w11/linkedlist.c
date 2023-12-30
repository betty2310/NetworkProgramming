#include "linkedlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// insert a link to head of linked list
void InsertFirst(account acc) {
    account *link = (account *) malloc(sizeof(account));
    strcpy(link->m_username, acc.m_username);
    strcpy(link->m_password, acc.m_password);
    link->m_status = acc.m_status;
    link->next = head;
    head = link;
}

// read data from file and add to linked list
void readData(char *filename) {
    account acc;
    FILE *f;
    // r == read file text
    if (!(f = fopen(filename, "r"))) {
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

// rewrite data after any change.
void rewriteData(char *filename) {
    FILE *f;
    if (!(f = fopen(filename, "w"))) {
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
