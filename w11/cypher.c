#include "cypher.h"

#include <stdio.h>
#include <string.h>

char *encode(char *message, int key) {
    char *encrypted = strdup(message);
    for (size_t i = 0; i < strlen(encrypted); i++) {
        encrypted[i] = encrypted[i] + key;
    }
    return encrypted;
}

char *decode(char *message, int key) {
    char *decrypted = strdup(message);
    for (size_t i = 0; i < strlen(decrypted); i++) {
        decrypted[i] = decrypted[i] - key;
    }
    return decrypted;
}