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


#define BUF_SIZE 1024
#define TRUE 1
#define FALSE 1
#define LISTEN_QUEUE 20
#define MAXLINE 1024
#define MAXBUF 8192

#define RIO_BUFSIZE 8192

extern char **environ;
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

void rio_readinitb(rio_t *rp, int fd);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

void read_requesthdrs(rio_t *rp);

void doit(int fd);

int listenToPort(int port);

ssize_t rio_writen(int fd, void *usrbuf, size_t n);

void get_filetype(char *filename, char *filetype);

void serv_static(int fd, char *filename, int filesize);

int parse_uri(char *uri, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

void serv_dynamic(int fd, char *filename, char *cgiargs);

int main(int argc, char **argv) {

    char hostname[64];
    char port[64];

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int connection = listenToPort(atoi(argv[1]));

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1) {
        int client_conn = accept(connection, (struct sockaddr *) &client_addr, &client_addr_size);
        getnameinfo((struct sockaddr *) &client_addr, client_addr_size, hostname, 64, port, 64, 0);
        if (client_conn == -1) {
            printf("accept() error");
        } else {
            printf("Connected client: %s %s \n", hostname, port);
        }
        doit(client_conn);
        close(client_conn);
    }

}

int listenToPort(int port) {
    int serv_sock;
    int option, optlen;

    struct sockaddr_in serv_adr;

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        printf("socket() error");
    }

    optlen = sizeof(option);
    option = TRUE;
    // 防止端口占用
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &option, optlen);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = PF_INET;//IPV4
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    if (bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1) {
        printf("bind() error");
    }

    if (listen(serv_sock, LISTEN_QUEUE) == -1) {
        printf("listen() error");
    }

    return serv_sock;

}

void doit(int fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")) {
        printf("method error");
        return;
    }

    // show request line and request header
    read_requesthdrs(&rio);

    /** Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    /** put file with filename in sbuf struct*/
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }
    /** serve static content */
    // S_ISREG(m)  is it a regular file?
    // S_ISDIR(m)  directory?
    // S_ISCHR(m)  character device?
    // S_ISBLK(m)  block device?
    // S_ISFIFO(m) FIFO (named pipe)?
    // S_ISLNK(m)  symbolic link?  (Not in POSIX.1-1996.)
    // S_ISSOCK(m) socket?  (Not in POSIX.1-1996.)
    if (is_static) { /** serve static file */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serv_static(fd, filename, sbuf.st_size);
    } else { /** serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serv_dynamic(fd, filename, cgiargs);
    }
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

ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
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

void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    /** Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /** Print the HTTP response*/
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    if (!strstr(uri, "cgi-bin")) { /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "home.html");
        }
        return 1;
    } else { /** Dynamic content */
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}


void serv_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /** Send response body to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    /** Send response body to client */
    srcfd = open(filename, O_RDONLY, 0);
    // PROT_READ  Pages may be read.
    //  MAP_PRIVATE
    //  Create a private copy-on-write mapping.  Updates to the
    //  mapping are not visible to other processes mapping the same
    //  file, and are not carried through to the underlying file.  It
    //  is unspecified whether changes made to the file after the
    //  mmap() call are visible in the mapped region.
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    // srcp points to area which contain the filename in the memory
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void serv_dynamic(int fd, char *filename, char *cgiargs) {

    char buf[MAXLINE], *emptylist[] = {NULL};

    /** Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web server\r\n");
    rio_writen(fd, buf, strlen(buf));

    if (fork() == 0) { // child process
        // The setenv() function adds the variable name to the environment with
        // the value value, if name does not already exist.  If name does exist
        // in the environment, then its value is changed to value if overwrite
        // is nonzero; if overwrite is zero, then the value of name is not
        // changed (and setenv() returns a success status).  This function makes
        // copies of the strings pointed to by name and value (by contrast with
        // putenv(3)).
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO); /** redirect stdout to client */
        execve(filename, emptylist, environ); /** Run CGI program */
    }
    wait(NULL);
}


/**
* get_filetype(char *filename,char *filetype)
*/
void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    } else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    } else if (strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    } else if (strstr(filename, ".jpg")) {
        strcpy(filetype, "image/jpeg");
    } else {
        strcpy(filetype, "text/plain");
    }
}

