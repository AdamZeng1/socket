//
// Created by adamzeng on 2019-10-06.
//

#ifndef UNTITLED2_HTTP_VERSION3_H
#define UNTITLED2_HTTP_VERSION3_H

void doit(int fd);

int read_requestdrs(rio_t *rp, char *content, char *requestMethod);

int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize, char *requestMethod);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs, char *requestMethod);

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *requestMethod);

void sig_child(int num);

int childEnded;

#endif //UNTITLED2_HTTP_VERSION3_H
