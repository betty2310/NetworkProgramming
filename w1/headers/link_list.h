#include "account.h"

typedef struct node {
 Account acc;
 struct node *next;
} node;

void push(node **head, Account acc);
Account* search(node *head, char *username);
void free_list(node *head);
