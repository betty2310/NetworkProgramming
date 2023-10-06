#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>

#define USAGE_MSG     "Usage: ./resolver <domain.name> or <ip>\n"
#define NOT_FOUND_MSG "Not found information\n"

void ip_to_hostname(char*, struct in_addr);
void hostname_to_ip(char*);

int
main(int argc, char** argv) {
    struct in_addr ip;

    if (argc != 2) {
        printf(USAGE_MSG);
        return 1;
    }

    if (!inet_pton(AF_INET, argv[1], &ip)) {
        hostname_to_ip(argv[1]);
    } else
        ip_to_hostname(argv[1], ip);
}

void
hostname_to_ip(char* host) {
    struct hostent* host_info;
    struct in_addr** addr_list;

    if ((host_info = gethostbyname(host)) == NULL) {
        printf(NOT_FOUND_MSG);
        return;
    }
    addr_list = (struct in_addr**) host_info->h_addr_list;
    printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
    printf("Alias IP:\n");
    for (int i = 1; addr_list[i] != NULL; i++) {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
}

void
ip_to_hostname(char* host, struct in_addr ip) {

    struct hostent* host_info;
    if ((host_info = gethostbyaddr((const void*) &ip, sizeof ip, AF_INET)) ==
        NULL) {
        printf(NOT_FOUND_MSG);
        return;
    }
    printf("Official name: %s\n", host_info->h_name);
    printf("Alias name:\n");
    for (int i = 0; host_info->h_aliases[i] != NULL; i++) {
        printf("%s\n", host_info->h_aliases[i]);
    }
}