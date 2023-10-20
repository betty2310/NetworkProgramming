#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

enum Status
{
    BLOCKED,
    ACTIVE
};

typedef struct Account
{
    char *username;
    char *password;
    enum Status status;
} Account;
