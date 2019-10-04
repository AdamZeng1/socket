//
// Created by adamzeng on 2019-09-25.
//

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/select.h>



/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

typedef struct sockaddr SA;

ssize_t rio_writen(int fd, void *usrbuf, size_t n);

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

void rio_readinitb(rio_t *rp, int fd);

int open_listenfd(int port);

void echo(int connfd);

void command(void);

#define LISTENQ  1024  /* second argument to listen() */
#define MAXLINE 8192


int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    fd_set read_set, ready_set;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(atoi(argv[1]));

    FD_ZERO(&read_set); /** Clear read set */
    FD_SET(STDIN_FILENO, &read_set); /** add stdin to read_set */
    FD_SET(listenfd, &read_set); /** add listenfd to read_set */

    while (1) {
        ready_set = read_set;
        select(listenfd + 1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set)) {
            command(); /** read command line from stdin */
        }
        if (FD_ISSET(listenfd, &ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
            echo(connfd);
            close(connfd);
        }
    }
}

void command(void) {
    char buf[MAXLINE];
    if (!fgets(buf, MAXLINE, stdin)) {
        exit(0);
    }
    printf("%s", buf);
}

int open_listenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *) &optval, sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);
    if (bind(listenfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;

    while (rp->rio_cnt <= 0) {  /* refill if buf is empty */
        // 读取rio_buf大小的数据,rio_cnt中存储的是读取到的字符数量
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
                           sizeof(rp->rio_buf));
        // 如果字符数量小于0
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) /* interrupted by sig handler return */
                return -1;
        }
            // 如果字符数量等于0
        else if (rp->rio_cnt == 0)  /* EOF */
            return 0;
        else // 将指针重置到buf头部
            rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { // iterate MAXLINE times
        if ((rc = rio_read(rp, &c, 1)) == 1) { // 从描述符中读取一个字符
            *bufp++ = c; // 存放到bufp当中
            if (c == '\n')
                break;
        } else if (rc == 0) { // 读取不到数据
            if (n == 1) // 完全未读取一些数据
                return 0; /* EOF, no data read */
            else // 读取到某一些数据
                break;    /* EOF, some data was read */
        } else
            return -1;      /* error */
    }
    *bufp = 0;
    return n;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)  /* interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else
                return -1;       /* errno set by write() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}


void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    rio_readinitb(&rio, connfd);
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server receive %d bytes\n", (int) n);
        rio_writen(connfd, buf, n);
    }
}

