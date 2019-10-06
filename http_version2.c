//
// Created by adamzeng on 2019-10-05.
//


/**
 * A simple, iterative HTTP/1.0 Web server that uses
 * the GET method to server static and dynamic content.
 *
 * concurrent server in multi process way
 */

#include "csapp.h"

void doit(int fd);

int read_requestdrs(rio_t *rp, char *content, char *requestMethod);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize, char *requestMethod);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs, char *requestMethod);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *requestMethod);

void sig_child(int num);

int childEnded;

int main(int argc, char **argv) {
    /** listen descriptor and conn descriptor */
    int listenfd, connfd;
    char hostname[MAX_LINE], port[MAX_LINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /** Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        fprintf(stderr, "use default port 5000\n");
        listenfd = Open_listenfd(5000);
    } else {
        listenfd = Open_listenfd(atoi(argv[1]));
    }

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen); // line:netp:tiny:accept
        getnameinfo((SA *) &clientaddr, clientlen, hostname, MAX_LINE, port, MAX_LINE, 0);
        if (connfd == -1) {
            fprintf(stderr, "accept() error\n");
        } else {
            fprintf(stdout, "Connected client: %s %s\n", hostname, port);
        }

        if (Fork() == 0) {
            Close(listenfd);
            doit(connfd);
            Close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

/**
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) {
    int is_static;
    /** get file status */
    struct stat sbuf;
    char buf[MAX_LINE], method[MAX_LINE], uri[MAX_LINE], version[MAX_LINE];
    rio_t rio;

    /** post method */
    int contentlen;
    char post_content[MAX_LINE];

    char filename[MAX_LINE], cgiargs[MAX_LINE];

    /** read request line and request header */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAX_LINE)) {
        return;
    }
    fprintf(stdout, "%s\n", buf);

    /** parse method uri version of request line */
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET") == 1 && strcasecmp(method, "POST") == 1 && strcasecmp(method, "HEAD") == 1) {
        clienterror(fd, method, "501", "Not Implemented", "Tiny server doesn't implement this method", method);
        return;
    }

    contentlen = read_requestdrs(&rio, post_content, method);

    /** Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);

    /** put file with filename in sbuf struct*/
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file", method);
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
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file", method);
            return;
        }
        serve_static(fd, filename, sbuf.st_size, method);
    } else { /** serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program", method);
            return;
        }
        if (strcasecmp(method, "POST") == 0) {
            strcpy(cgiargs, post_content);
        }
        serve_dynamic(fd, filename, cgiargs, method);
    }
}

void serve_static(int fd, char *filename, int filesize, char *requestMethod) {
    int srcfd;
    char *srcp, filetype[MAX_LINE], buf[MAXBUF];

    /** Send response body to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    if (!strcasecmp(requestMethod, "HEAD")) {
        return;
    }

    /** Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    // PROT_READ  Pages may be read.
    //  MAP_PRIVATE
    //  Create a private copy-on-write mapping.  Updates to the
    //  mapping are not visible to other processes mapping the same
    //  file, and are not carried through to the underlying file.  It
    //  is unspecified whether changes made to the file after the
    //  mmap() call are visible in the mapped region.
//    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    srcp = (char *) malloc(sizeof(char) * filesize);
    // srcp points to area which contain the filename in the memory
    Rio_readn(srcfd, srcp,
              filesize); // read from file pointer srcfd to char pointer srcp which is allocated in memory before
    close(srcfd);
    Rio_writen(fd, srcp, filesize);
//    munmap(srcp, filesize);
    free(srcp);
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *requestMethod) {

    char buf[MAX_LINE], *emptylist[] = {NULL};
    struct sigaction sigact;
//    sigact.sa_handler = sig_child;//信号响应函数
//    sigact.sa_flags = 0;//选择第一种函数类型(根据 struct sigaction 结构体来的)
//    sigemptyset(&sigact.sa_mask);//置0,SIGCHLD 默认是忽略的
//    sigaction(SIGCHLD, &sigact, NULL);  //注册信号
    signal(SIGCHLD, sig_child);

    /** Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web server\r\n");
    rio_writen(fd, buf, strlen(buf));

    if (!strcasecmp(requestMethod, "HEAD")) {
        return;
    }


    setenv("QUERY_STRING", cgiargs, 1);
    printf("%s\n", cgiargs);

    if (fork() == 0) { // child process
        // The setenv() function adds the variable name to the environment with
        // the value value, if name does not already exist.  If name does exist
        // in the environment, then its value is changed to value if overwrite
        // is nonzero; if overwrite is zero, then the value of name is not
        // changed (and setenv() returns a success status).  This function makes
        // copies of the strings pointed to by name and value (by contrast with
        // putenv(3)).
        setenv("QUERY_STRING", cgiargs, 1);
        printf("%s\n", cgiargs);
        dup2(fd, STDOUT_FILENO); /** redirect stdout to client */
        execve(filename, emptylist, environ); /** Run CGI program */
    }
    while (!childEnded) {};
    return;
}

int read_requestdrs(rio_t *rp, char *content, char *requestMethod) {
    char buf[MAX_LINE];
    int contentLength = 0;

    Rio_readlineb(rp, buf, MAX_LINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAX_LINE);
//        printf("%s\n", buf);
        if (strcasecmp(requestMethod, "POST") == 0 && strstr(buf, "Content-Length: ") == buf) {
            contentLength = atoi(buf + strlen("Content-Length: "));
            printf("%d\n", contentLength);
        }
    }

    if (strcasecmp(requestMethod, "POST") == 0) {
        contentLength = (int) rio_readnb(rp, content, (size_t) contentLength);
        content[contentLength] = '\0';
        printf("%d\n", contentLength);
    }

    return contentLength;

}

int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;
    /** static content */
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "home.html");
            return 1;
        }
    } else { /** dynamic content */
        ptr = index(uri, '?'); // get index of '?' in uri
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *requestMethod) {
    char buf[MAX_LINE], body[MAXBUF];

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
    if (strcasecmp(requestMethod, "HEAD") != 0) {
        rio_writen(fd, body, strlen(body));
    }
}

void sig_child(int num) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { // recycling child process using unblock way
        if (WIFEXITED(status)) {
            printf("child %d exit %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("child %d cancel signal %d\n", pid, WTERMSIG(status));
        }
    }
    childEnded = 1;
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
    } else if (strstr(filename, ".mpg")) {
        strcpy(filetype, "video/mpeg");
    } else {
        strcpy(filetype, "text/plain");
    }
}
