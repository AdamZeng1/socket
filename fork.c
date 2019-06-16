//
// Created by adamzeng on 2019-06-16.
//

#include <stdio.h>
#include <unistd.h>

int gval = 10;

int main(int argc, char **argv) {
    pid_t pid;
    int lval = 20;
    gval++, lval += 5;

    pid = fork();
    if (pid == 0) { // 子进程执行
        gval += 2, lval += 2;
    } else {
        gval -= 2, lval -= 2;
    }

    if (pid == 0) {
        printf("Child Proc:[%d,%d]\n", gval, lval);
    } else {
        printf("Parent Proc:[%d,%d]\n", gval, lval);
    }
    return 0;
}