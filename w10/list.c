#include "list.h"

#include <stdio.h>
#include <string.h>

// print all linked list
void print_List() {
    account *ptr = head;
    printf("\n");
    while (ptr != NULL) {
        printf("Name: %-15s - Pass: %-10s - Status:%d - IsLogin:%d - NumOfFalseLogin:%d\n",
               ptr->m_username, ptr->m_password, ptr->m_status, ptr->m_isLogin,
               ptr->m_numOfFalseLogin);
        ptr = ptr->next;
    }
}

// insert a link to head of linked list
void insert(account acc) {
    account *link = (account *) malloc(sizeof(account));
    strcpy(link->m_username, acc.m_username);
    strcpy(link->m_password, acc.m_password);
    link->m_status = acc.m_status;
    link->next = head;
    head = link;
}