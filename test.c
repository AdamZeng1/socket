//
// Created by adamzeng on 2019-10-06.
//

#include <stdio.h>
#include <memory.h>

int main() {
    const char haystack[20] = "TutorialPoint";
    const char needle[10] = "Point";
    char *ret;

    ret = strstr(haystack, needle);
    printf("The substring is: %s\n", ret);

    return 0;
}