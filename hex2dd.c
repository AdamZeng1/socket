//
// Created by adamzeng on 2019-09-13.
//

#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

struct sockaddr_in sa;
char str[INET_ADDRSTRLEN];
struct in_addr s1;
char *s;

int main(int argc, char *argv[]) {
    // get hexdecimal number from commandline and store it into struct in_addr.s_addr
    s = argv[1];
    int i;
    i = strtol(s, NULL, 16);
    s1.s_addr = i;

    // convert it to dot and number format and store it into char[] str
    inet_ntop(AF_INET, (void *) &s1, str, INET_ADDRSTRLEN);

    printf("%s\n", str);
}