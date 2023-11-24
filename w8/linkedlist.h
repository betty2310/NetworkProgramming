#ifndef __LINKED_LIST__
#define __LINKED_LIST__

#define MAX 256

typedef struct tagAccount {
    char m_username[MAX];
    char m_password[MAX];
    int m_status;
    int m_isLogin;
    int m_numOfFalseLogin;
    struct tagAccount *next;
} account;

extern account *head;
extern account *current;

void InsertFirst(account acc);
void readData(char *filename);
void rewriteData(char *filename);

#endif   // !__LINKED_LIST__
