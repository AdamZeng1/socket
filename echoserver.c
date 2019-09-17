//
// Created by adamzeng on 2019-09-14.
//

#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in
    struct sockaddr_storage clientaddr; /** Enough space for any address */
    char client_hostname[MAX_LINE], client_port[MAX_LINE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_LINE, client_port, MAX_LINE, 0);
        printf("Connected to (%s,%s)\n", client_hostname, client_port);
        echo(connfd);
    }
}

void echo(int connfd) {
    size_t n;
    char buf[MAX_LINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAX_LINE)) != 0) {
        printf("server received %d bytes\n", (int) n);
        Rio_writen(connfd, buf, n);
    }
}