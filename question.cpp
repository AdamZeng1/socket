//
// Created by adamzeng on 2019-09-18.
//

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>

using namespace std;

bool cmp(int a, int b) {
    return a < b;
}

int main() {
    int hash1[250] = {0};
    int hash2[250] = {0};
    int res1[250] = {0};
    int res2[250] = {0};
    char string1[100] = {'\0'}, string2[100] = {'\0'}, string[200] = {'\0'};
    scanf("%s", string);
    int index = -1;
    for (int i = 0; i < 200; ++i) {
        if (string[i] == ';') {
            index = i;
            break;
        }
        string1[i] = string[i];

    }
    for (int i = index + 1; i < 200; ++i) {
        if (string[i] == '\0') {
            break;
        }
        string2[i - index - 1] = string[i];
    }

    for (int i = 0; i < 100; ++i) {

        hash1[string1[i]]++;
    }

    for (int i = 0; i < 100; ++i) {

        hash2[string2[i]]++;
    }

    int num1 = 0;
    for (int i = 0; i < 250; ++i) {
        if (hash1[i] != 0) {
            res1[num1++] = hash1[i];
        }
    }
    int num2 = 0;
    for (int i = 0; i < 250; ++i) {
        if (hash2[i] != 0) {
            res2[num2++] = hash2[i];
        }
    }
    sort(res1, res1 + num1);
    sort(res2, res2 + num1);

    for (int i = 0; i < 250; ++i) {
        if (res1[i] != res2[i]) {
            printf("False");
        }
    }
    printf("True");

    return 0;
}
