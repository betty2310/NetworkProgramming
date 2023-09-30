#include "link_list.h"

node *read_file(char *file_name, node **head);
void append_account_to_file(char *file_name, Account acc);
void change_account_status(char *file_name, Account *acc, enum Status status);
