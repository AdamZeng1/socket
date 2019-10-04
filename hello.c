//
// Created by adamzeng on 2019-09-26.
//

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thread(void *vargp);

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, thread, NULL);
    pthread_join(tid, NULL);
    exit(0);
}

void *thread(void *vargp) { /** Thread routine */
    printf("Hello, World\n");
    return NULL;
}
