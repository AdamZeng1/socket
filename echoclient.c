//
// Created by adamzeng on 2019-09-14.
//

//
// Created by adamzeng on 2019-09-14.
//
#include "csapp.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port, buf[MAX_LINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    // client build connection with server
    clientfd = Open_clientfd(host, *port);
    Rio_readinitb(&rio, clientfd);

    // read context from stdin, if it is not NULL
    while (Fgets(buf, MAX_LINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAX_LINE);
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}