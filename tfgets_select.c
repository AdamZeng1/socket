//
// Created by adamzeng on 2019-10-03.
//

#include "csapp.h"


char *tfgets(char *s, int size, FILE *stream);

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
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /** wait up to five second */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1) {
        perror("select()");
    } else if (retval) {
        return fgets(s, size, stream);
    } else {
        return NULL;
    }

    return 0;
}

