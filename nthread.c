//
// Created by adamzeng on 2019-10-01.
//

#include "csapp.h"

void *thread(void *vargp);

int main(int argc, char **argv) {
    pthread_t tid;

    for (int i = 0; i < atoi(argv[1]); ++i) {
        Pthread_create(&tid, NULL, thread, NULL);
    }
    Pthread_join(tid, NULL);
    exit(0);
}


/** Thread routine */
void *thread(void *vargp) {
    printf("Hello World\n");
    return NULL;
}

