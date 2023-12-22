#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

#define MAX 256

typedef struct Account {
    char m_username[MAX];
    char m_password[MAX];
    int m_status;
    int m_isLogin;
    int m_numOfFalseLogin;
    struct Account *next;
} account;

extern account *head;
extern account *current;

void print_List();

void insert(account acc);

#endif
