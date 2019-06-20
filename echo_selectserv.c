//
// Created by adamzeng on 2019-06-18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 32

void error_handling(char *message);

int main(int argc, char **argv) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout, timeout_init;
    fd_set reads, reads_init;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }
    if (listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    FD_ZERO(&reads_init);
    FD_SET(serv_sock, &reads_init);
    FD_SET(0, &reads_init);// stdin also works
    fd_max = serv_sock;

    timeout_init.tv_sec = 5;
    timeout_init.tv_usec = 0;

    while (1) {
        reads = reads_init; // recover initial fd_set reads
        timeout = timeout_init;
        if ((fd_num = select(fd_max + 1, &reads_init, 0, 0, &timeout)) == -1) {
            break; // error occur
        }
        if (fd_num == 0) { // overtime
            continue;
        }
        for (int i = 0; i < fd_max + 1; ++i) { // iterate every bits in fd_set, if it's set to a file descriptor before
            if (FD_ISSET(i, &reads)) {     // judge if it's serv_sock or clnt_sock
                if (i == serv_sock) { // serv_sock change, new client requires to connect
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_adr, &adr_sz);
                    if (clnt_sock == -1) {
                        error_handling("accept() error");
                    }
                    FD_SET(clnt_sock, &reads_init); // register new clnt_sock to fd_set reads
                    if (fd_max < clnt_sock) {
                        fd_max = clnt_sock;
                    }
                    printf("connected client: %d \n", clnt_sock);
                }
            } else { // clnt_sock event, read message from clnt_sock
                str_len = read(i, buf, BUF_SIZE);
                if (str_len) { // echo to client
                    buf[str_len] = 0;
                    printf("Message from client %d: %s", i, buf);
                    write(i, buf, str_len);
                } else {
                    FD_CLR(i, &reads_init); // clear all fd_set reads
                    close(i);
                    printf("close client: %d \n", i);
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}


void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
