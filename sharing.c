//
// Created by adamzeng on 2019-09-27.
//


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


#define N 2

void *thread(void *vargp);

char **ptr; /** Global variable */

int main() {
    int i;
    pthread_t tid;
    char *msgs[N] = {
            "Hello from foo",
            "Hello from bar"
    };

    ptr = msgs;
    for (int i = 0; i < N; ++i) {
        pthread_create(&tid, NULL, thread, (void *) i);
    }
    pthread_exit(NULL);
}

void *thread(void *vargp) {
    int myid = (int) vargp;
    static int cnt = 0;
    printf("[%d]: %s (cnt=%d)\n", myid, ptr[myid], ++cnt);
    return NULL;
}

