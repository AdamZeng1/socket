//
// Created by adamzeng on 2019-10-01.
//

#include "csapp.h"

void *thread(void *vargp);

int main() {
    pthread_t tid;

    Pthread_create(&tid, NULL, thread, NULL);

    Pthread_join(tid, NULL);
    exit(0);
}

/** Thread routine */
void *thread(void *vargp) {
    Sleep(1);
    printf("Hello World!\n");
    return NULL;
}
