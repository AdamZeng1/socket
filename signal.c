//
// Created by adamzeng on 2019-06-16.
//

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// handler
void timeout(int sig) {
    if (sig == SIGALRM) {
        puts("Time out!");
    }
    alarm(2);
}

// handler
void keycontrol(int sig) {
    if (sig == SIGINT) {
        puts("CTRL+C pressed");
    }
}

int main(int argc, char **argv) {
    int i;
    signal(SIGALRM, timeout);
    signal(SIGINT, keycontrol);
    // produce SIGALRM signal, its time unit is second
    alarm(2);

    for (int i = 0; i < 3; ++i) {
        puts("wait...");
        sleep(100);
    }
    return 0;
}