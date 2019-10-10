//
// Created by adamzeng on 2019-09-19.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAXLINE 8192

int parseNum(char *s) {
    int i = strlen(s) - 1;
    while (i > 0 && s[i - 1] >= '0' && s[i - 1] <= '9')
        i--;
    return atoi(s + i);
}

int main() {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;
    /** Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0'; // 将 & 字符替换成 \0, 这样读取的时候就可以直接用\0来终止读取
        strcpy(arg1, buf);
        strcpy(arg2, p + 1); // 将char* p指针指向 & 后一个字符

        n1 = parseNum(arg1);
        n2 = parseNum(arg2);
    }

    /** Make the response body */
    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sThe internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);

    /** generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int) strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
