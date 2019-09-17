//
// Created by adamzeng on 2019-06-16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
#define TRUE 1
#define FALSE 1

void error_handling(char *message);

void read_childproc(int sig);

int main(int argc, char **argv) {
    pid_t pid;
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int str_len, state, optlen, option;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    struct sigaction act;


    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_handling("socket() error");
    }

    optlen = sizeof(option);
    option = TRUE;
    // 防止端口占用
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &option, optlen);

    // server address struct
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

    while (1) { /** running forever in this block of code */
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_adr, &adr_sz);
        if (clnt_sock == -1) {
            continue;
        } else {
            puts("new client connected");
        }
        pid = fork();
        if (pid == -1) {
            close(clnt_sock);
            continue;
        }
        if (pid == 0) { /** child process area */
            close(serv_sock);
            /** child process read message from clnt_sock */
            while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0) {
                write(clnt_sock, message, str_len);
                puts(message);
            }

            close(clnt_sock);
            puts("client disconnected...");
            return 0; /** child process exit code is 0 */
        } else { /** main process */
            close(clnt_sock); /** close main process input stream */
        }
    }
    close(serv_sock); /** close main process output stream */
    return 0;
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id : %d \n", pid);
}


void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}