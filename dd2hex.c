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
    printf("%s\n", argv[1]);
    inet_pton(AF_INET, argv[1], (void *) &s1);
    printf("hexdecimal of ip address: 0x%x\n", s1.s_addr);
}