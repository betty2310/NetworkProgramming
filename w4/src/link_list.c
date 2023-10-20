#include "../headers/link_list.h"

#include <stdlib.h>
#include <string.h>

void push(node **head, Account acc)
{
    node *new_node = (node *)malloc(sizeof(node));
    new_node->acc = acc;
    new_node->next = *head;
    *head = new_node;
}

Account *search(node *head, char *username)
{
    node *current = head;
    while (current != NULL)
    {
        if (strcmp(current->acc.username, username) == 0)
        {
            return &current->acc;
        }
        current = current->next;
    }
    return NULL;
}

void free_list(node *head)
{
    node *tmp;
    while (head != NULL)
    {
        tmp = head;
        head = head->next;
        free(tmp->acc.username);
        free(tmp->acc.password);
        free(tmp);
    }
}
