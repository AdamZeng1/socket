//
// Created by adamzeng on 2019-09-26.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RIO_BUFSIZE 8192
#define LISTENQ  1024  /* second argument to listen() */
#define MAXLINE 8182

typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

typedef struct { /** Represents a pool of connected descriptors */
    int maxfd; /** Largest descriptor in read_set */
    fd_set read_set; /** Set of all active descriptors */
    fd_set ready_set; /** Subset of descriptors ready for reading */
    int nready; /** Number of descriptors from select */
    int maxi; /** High water index into client array */
    int clientfd[FD_SETSIZE]; /** Set of active descriptors */
    rio_t clientrio[FD_SETSIZE]; /** Set of active read buffers */
} pool;

typedef struct sockaddr SA;

void check_clients(pool *p);

void init_pool(int listenfd, pool *p);

int open_listenfd(int port);

void add_client(int connfd, pool *p);

ssize_t rio_writen(int fd, void *usrbuf, size_t n);

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

void rio_readinitb(rio_t *rp, int fd);

int open_listenfd(int port);

int byte_cnt = 0; /** Counts total bytes received by server */

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    static pool pool;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = open_listenfd(atoi(argv[1]));
    init_pool(listenfd, &pool);

    while (1) {
        /** wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);
        printf("nready: %d\n",pool.nready);

        /** if listening descriptor ready, add new client to the pool */
        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
            add_client(connfd, &pool);
        }
        /** Echo a text line from each ready connected descriptor */
        check_clients(&pool);
    }
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

void init_pool(int listenfd, pool *p) {
    /** Initially, there are no connected descriptor */
    int i;
    p->maxi = -1;
    /** Initially, all element in clientfd array is zero */
    for (i = 0; i < FD_SETSIZE; ++i) {
        p->clientfd[i] = -1;
    }

    /** Initially, listenfd is only member of select read set */
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p) {
    int i;
    p->nready--;
    /** iterate the clientfd array */
    for (i = 0; i < FD_SETSIZE; ++i) { /** find a available slot */
        if (p->clientfd[i] < 0) {
            /** Add descriptor to descriptor pool */
            p->clientfd[i] = connfd;
            rio_readinitb(&p->clientrio[i], connfd);
            /** Add the descriptor to the descriptor set */
            FD_SET(connfd, &p->read_set);
            /** Update max descriptor and pool high water mark */
            if (connfd > p->maxfd) {
                p->maxfd = connfd;
            }
            /** max index in clientfd */
            if (i > p->maxi) {
                p->maxi = i;
            }
            break;
        }
    }
    if (i == FD_SETSIZE) { /** Couldn't find an empty slot */
        printf("add_client error: Too many clients");
    }
}

void check_clients(pool *p) {
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

//    printf("maxi: %d\n",p->maxi);
//    printf("nready: %d\n",p->nready);

    /** nready > 0 means already have file descriptor in ready set */
    for (i = 0; (i <= p->maxi) && (p->nready > 0); ++i) {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        /** If the descriptor is ready, echo a text from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                byte_cnt += n;
                printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
                printf("message from client %d: %s", connfd, buf);
                rio_writen(connfd, buf, (size_t) n);
            } else { /** EOF detected, remove descriptor from pool */
                close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}



