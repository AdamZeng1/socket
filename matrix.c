//
// Created by adamzeng on 2019-10-04.
//

#include "csapp.h"

#include "matrix.h"

int M1[N][M];
int M2[N][M];

int MUL12[N][M] = {0};

void non_concurrent_mul(void) {
    int i, j, k;
    for (i = 0; i < N; ++i) { /** iterate M1 rows */
        for (j = 0; j < N; ++j) { /** iterate M2 columns */
            int sum = 0;
            for (k = 0; k < M; ++k) { /** iterate M1 columns and M2 rows*/
                sum += M1[i][k] * M2[k][j];
            }
            MUL12[i][j] = sum;
        }
    }
}

void *thread_mul(void *vargp) {
    int idx = *(int *) vargp;
    int start = ROWS_PER_THREAD * idx; /** place where compute start from */
    int i, j, k;
    for (i = start; i < start + ROWS_PER_THREAD; i++) { /** iterate from start and end in another ROWS_PER_THREAD */
        for (j = 0; j < N; ++j) { /** iterate M2 columns */
            int sum = 0;
            for (k = 0; k < M; ++k) {
                sum += M1[i][k] * M2[k][j];
            }
            MUL12[i][j] = sum;
        }
    }
}

void concurrent_mul(void) {
    pthread_t tid[THREAD];
    int param[THREAD];
    int i;

    for (i = 0; i < THREAD; ++i) {
        param[i] = i;
        Pthread_create(&tid[i], NULL, thread_mul, &param[i]);
    }
    for (i = 0; i < THREAD; ++i) {
        Pthread_join(tid[i], NULL);
    }
}

int main() {

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            M1[i][j] = 1;
            M2[i][j] = 2;
        }
    }
//    concurrent_mul();
    non_concurrent_mul();
    printf("hello world");

    return 0;
}
