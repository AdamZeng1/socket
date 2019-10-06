//
// Created by adamzeng on 2019-10-05.
//

#include "csapp.h"


char *tfgets(char *s, int size, FILE *stream);

void *thread(void *vargp);

char *threadBuf = NULL;
int timeout = -1;

typedef struct {
    char *s;
    int size;
    FILE *stream;
} fgetstruct;

void *thread_read(void *vargp) {
    char *s = ((fgetstruct *) vargp)->s;
    int size = ((fgetstruct *) vargp)->size;
    FILE *stream = ((fgetstruct *) vargp)->stream;
    threadBuf = fgets(s, size, stream);
    timeout = 0;
}

void *thread_sleep(void *vargp) {
    Sleep(5);
    timeout = 1;
}

int main() {
    char buf[MAX_LINE];

    if (tfgets(buf, MAX_LINE, stdin) == NULL) {
        printf("BOOM!\n");
    } else {
        printf("%s", buf);
    }

    return 0;
}


char *tfgets(char *s, int size, FILE *stream) {
    pthread_t tid_sleep, tid_read;
    fgetstruct fgetstruct;
    fgetstruct.s = s;
    fgetstruct.size = size;
    fgetstruct.stream = stream;
    Pthread_create(&tid_read, NULL, thread_read, &fgetstruct);
    Pthread_create(&tid_sleep, NULL, thread_sleep, NULL);

    /** barrier: wait until any thread finish */
    while (timeout == -1) {
    }

    if (timeout == 1) {
        Pthread_cancel(tid_read);
        return NULL;
    } else {
        Pthread_cancel(tid_sleep);
        return threadBuf;
    };

}

/** Thread Routine */
void *thread(void *vargp) {
    char *s = ((fgetstruct *) vargp)->s;
    int size = ((fgetstruct *) vargp)->size;
    FILE *stream = ((fgetstruct *) vargp)->stream;
    threadBuf = fgets(s, size, stream);
}

