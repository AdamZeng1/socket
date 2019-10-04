//
// Created by adamzeng on 2019-09-29.
//

#include "sbuf.h"
#include "csapp.h"

#define NTHREADS 4
#define SBUFSIZE 16

#define MAXLINE 8182

typedef struct sockaddr SA;

static int byte_cnt; /** Byte counter */

static sem_t mutex; /** and mutex protects it */

void echo_cnt(int connfd);

void *thread(void *vargp);

static void init_echo_cnt(void);

sbuf_t sbuf; /* shared buffer of connected descriptors */

int main(int argc, char **argv) {
    int i, listenfd, connfd, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE); //line:conc:pre:initsbuf
    listenfd = Open_listenfd(port);


    for (i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
        Pthread_create(&tid, NULL, thread, NULL);               //line:conc:pre:endcreate

    while (1) {
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
    }
}

void *thread(void *vargp) {
    Pthread_detach(pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
        echo_cnt(connfd);                /* Service client */
        Close(connfd);
    }
}

/* $end echoservertpremain */

static void init_echo_cnt(void) {
    sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void echo_cnt(int connfd) {
    int n;
    char buf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, init_echo_cnt);
    printf("%d", (int) mutex);
    rio_readinitb(&rio, connfd);
//    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
//        P(&mutex);
//        byte_cnt += n;
//        printf("server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
//        V(&mutex);
//        rio_writen(connfd, buf, n);
//    }
}
