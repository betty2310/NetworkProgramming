
#define MAX 256

typedef struct tagAccount {
    char m_username[MAX];
    char m_password[MAX];
    int m_status;
    int m_isLogin;
    int m_numOfFalseLogin;
    struct tagAccount *next;
} account;

