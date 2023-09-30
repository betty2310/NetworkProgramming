enum Status {
    BLOCKED,
    ACTIVE
};

typedef struct Account {
    char* username;
    char* password;
    enum Status status;
} Account;

void getAccount(Account *acc);

Account *setAccount(char *username, char *password, enum Status status);