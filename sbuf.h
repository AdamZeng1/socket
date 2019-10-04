//
// Created by adamzeng on 2019-09-28.
//
#ifndef UNTITLED2_SBUF_H
#define UNTITLED2_SBUF_H

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
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct {
    int *buf; /** Buffer array */
    int n; /** Maximum number of slots */
    int front; /** buf[(front+1)%n] is the first item */
    int rear; /** buf[rear%n] is the last item */
    sem_t mutex; /** protects accesses to buf */
    sem_t slots; /** counts available slots */
    sem_t items; /** counts available items */
} sbuf_t;


void sbuf_init(sbuf_t *sp, int n);

void sbuf_insert(sbuf_t *sp, int item);

int sbuf_remove(sbuf_t *sp);

void P(sem_t *sem);

void V(sem_t *sem);

void unix_error(char *msg);

#endif //UNTITLED2_SBUF_H
