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
#include <semaphore.h>

void *thread(void *vargp); /** Thread routine prototype */

void unix_error(char *msg);

void P(sem_t *sem);

void V(sem_t *sem);

volatile long cnt = 0; /** Counter */
sem_t mutex; /** mutex = 1 */

int main(int argc, char **argv) {
    sem_init(&mutex, 0, 1); /** mutex = 1 */
    long niters;
    pthread_t tid1, tid2;

    /** Check input arguments */
    if (argc != 2) {
        printf("usage: %s <niters>\n", argv[0]);
        exit(0);
    }
    niters = atoi(argv[1]);

    /** Create threads and wait for them to finish */
    pthread_create(&tid1, NULL, thread, &niters);
    pthread_create(&tid2, NULL, thread, &niters);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    /** Check result */
    if (cnt != 2 * niters) {
        printf("Boom! cnt=%ld\n", cnt);
    } else {
        printf("OK cnt=%ld\n", cnt);
    }
    exit(0);
}

/** Thread routine */

void *thread(void *vargp) {
    long i, niters = *((long *) vargp);

    for (i = 0; i < niters; ++i) {
        sem_wait(&mutex);
//        P(&mutex);
        cnt++; /** 由于全局变量mutex的作用 */
//        V(&mutex);
        sem_post(&mutex);
    }

    return NULL;
}

void P(sem_t *sem) {
    if (sem_wait(sem) < 0)
        unix_error("P error");
}

void V(sem_t *sem) {
    if (sem_post(sem) < 0)
        unix_error("V error");
}

void unix_error(char *msg) /* unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}