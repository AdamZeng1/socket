//
// Created by adamzeng on 2019-09-30.
//
#include "csapp.h"

#define N 4

void *thread(void *vargp);

int main() {
    pthread_t tid[N];
    int i, *ptr, myid[N];

    for (i = 0; i < N; ++i) {
        // allocate a space in the memory whose size is one integer size, return pointer points to this space
//        ptr = Malloc(sizeof(int));
//        *ptr = i;
        myid[i] = i;
        Pthread_create(&tid[i], NULL, thread, &myid[i]);
    }
    for (i = 0; i < N; ++i) {
        Pthread_join(tid[i], NULL);
    }
    exit(0);
}

/** Thread routine */
void *thread(void *vargp) {
    int myid = *((int *) vargp);
//    Free(vargp);
    printf("Hello from thread %d\n", myid);
    return NULL;
}

//if (i<N) {
//
//Pthread_create(&tid[i], NULL, thread, &i); /** 这里是传递的指向i的指针,不是值传递 */
///**
// * 在子线程执行的时候执行到
// * int myid = *((int *) vargp); 这段代码的时候取到的是指向i的指针,
// * 这个时候i++,myid就被赋值为更大的i,不是原本想要传递的i值了
// *
// * */
//i++; /** 主线程执行到i++后面时,子线程说不定已经执行完int myid = *((int *) vargp); 也可能没执行到int myid = *((int *) vargp);
// * 如果子线程执行完int myid = *((int *) vargp);
// * 那myid取到的是i++之前的值.
// * 如果子线程执行没执行到int myid = *((int *) vargp);
// * 那myid取到的是i++之后的值.
// * */
//}