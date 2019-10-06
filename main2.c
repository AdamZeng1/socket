//
// Created by adamzeng on 2019-10-06.
//

#include "csapp.h"
#include "pool.h"
#include "tiny.h"


void *thread(void *vargp);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    char hostname[MAX_LINE], port[MAX_LINE];
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    int *connfdp;


    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        fprintf(stderr, "use default port 5000\n");
        listenfd = Open_listenfd(atoi("5000"));
    } else {
        listenfd = Open_listenfd(atoi(argv[1]));
    }


    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        getnameinfo((SA *) &clientaddr, clientlen, hostname, MAX_LINE, port, MAX_LINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        connfdp = (int *) Malloc(sizeof(int));
        *connfdp = connfd;
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

void *thread(void *vargp) {
    int connfd = *(int *) vargp;
    Pthread_detach(Pthread_self());
    Free(vargp);

    doit(connfd);
    Close(connfd);
    return NULL;
}
