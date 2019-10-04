//
// Created by adamzeng on 2019-09-30.
//

#include "csapp.h"

#define MAXTHREADS 32

void *sum_mutex(void *vargp);/** Thread routine */

/** Global shared Variable */
long gsum = 0; /** Global sum */
long nelems_pre_thread; /** Number of elements to sum */
sem_t mutex; /** Mutex to protect global sum */


int main(int argc, char **argv) {
    long i, nelems, log_nelems, nthreads, myid[MAXTHREADS];
    pthread_t tid[MAXTHREADS];

    /** Get input arguments */
    if (argc != 3) {
        printf("Usage: %s <nthreads> <log_nelems>\n", argv[0]);
        exit(0);
    }
    nthreads = atoi(argv[1]);
    log_nelems = atoi(argv[2]);
    nelems = (1L << log_nelems);
    nelems_pre_thread = nelems / nthreads;
    sem_init(&mutex, 0, 1);

    /** Create peer threads and wait for them to finish */
    for (int i = 0; i < nthreads; ++i) {
        myid[i] = i;
        Pthread_create(&tid[i], NULL, sum_mutex, &myid[i]);
    }
    for (int i = 0; i < nthreads; ++i) {
        Pthread_join(tid[i], NULL);
    }
    /** Check final answer */
    if (gsum != (nelems * (nelems - 1)) / 2) {
        printf("Error: result=%ld\n", gsum);
    }
    exit(0);
}

/** Thread routine for psum_mutex.c */
void *sum_mutex(void *vargp) {
    long myid = *((long *) vargp); /** Extract the thread ID */
    long start = myid * nelems_pre_thread; /** Start element index */
    long end = start + nelems_pre_thread;  /** End element index */
    long i;

    for (i = start; i < end; ++i) {
        P(&mutex);
        gsum += 1;
        V(&mutex);
    }
    return NULL;
}

/** Thread routine for psum_array.c */
void *sum_array(void *vargp) {
    long myid = *((long *) vargp); /** Extract the thread ID */
    long start = myid * nelems_pre_thread; /** Start element index */
    long end = start + nelems_pre_thread;  /** End element index */
    long i;

    for (i = start; i < end; ++i) {

    }
    return NULL;
}