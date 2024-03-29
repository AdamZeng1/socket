//
// Created by adamzeng on 2019-06-16.
//

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    pid_t pid = fork();

    if (pid == 0) { // if Child process
        puts("Hi, I am a child process");
    } else {
        printf("Child process ID: %d \n", pid);
        sleep(30);
    }

    if (pid == 0) {
        puts("End child process");
    } else {
        puts("End parent process");
    }
    return 0;


}