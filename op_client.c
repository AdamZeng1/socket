//
// Created by adamzeng on 2019-06-10.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4

void error_handling(char *message);

int main(int argc, char **argv) {
    int sock;
    char opmsg[BUF_SIZE]; // 用于转存的地址空间
    int result, opnd_cnt, i;
    struct sockaddr_in serv_adr; // 存放连接地址的地方

    int str_len, recv_len, recv_cnt;

    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error_handling("socket() error");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("connect() error!");
    } else {
        puts("Connected............");
    }

    // 进入协议处理阶段,此时客户端与服务端已经建立连接
    fputs("Operand count: ", stdout);
    scanf("%d", &opnd_cnt);
    opmsg[0] = (char) opnd_cnt;

    for (int i = 0; i < opnd_cnt; ++i) {
        printf("Operand %d: ", i + 1);
        scanf("%d", (int *) &opmsg[i * OPSZ + 1]); // 指针操作,每次移动OPSZ距离,4 bytes的integer的长度
    }
    fgetc(stdin);
    fputs("Operator: ", stdout);
    scanf("%c", &opmsg[opnd_cnt * OPSZ + 1]); // 读取操作符
    write(sock, opmsg, opnd_cnt * OPSZ + 2); // 写入到socket流当中,最后一个参数是写入长度
    read(sock, &result, RLT_SIZE); // 读取服务端返回的结果值到result中

    printf("Operation result: %d \n", result);
    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

