//
// Created by adamzeng on 2019-09-16.
//

#include<stdio.h>
#include <memory.h>

int isLeafYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    int year, month, day;
    int days = 0;
    scanf("%d %d %d", &year, &month, &day);
    int hash[14];
    memset(hash, 0, sizeof(hash));
    hash[1] = 31;
    if (isLeafYear(year)) {
        hash[2] = 29;
    } else {
        hash[2] = 28;
    }
    hash[3] = 31;
    hash[4] = 30;
    hash[5] = 31;
    hash[6] = 30;
    hash[7] = 31;
    hash[8] = 31;
    hash[9] = 30;
    hash[10] = 31;
    hash[11] = 30;
    hash[12] = 31;
    // month = 2
    for (int i = 1; i <= month - 1; ++i) {
        days += hash[i];
    }

    days += day;
    printf("%d", days);
    return 0;
}