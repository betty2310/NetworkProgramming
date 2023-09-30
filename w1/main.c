#include "strings.h"
#include "account.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    int c;
    do {
        printf("%s", MENU);
        c = getchar();
    } while (c > '0' && c < '5');
//    Account *account = (Account *) malloc(sizeof(Account));
//    account->status = ACTIVE;
//    printf("%d", account->status);

    return 0;
}
