#ifndef __RIO_H
#define __RIO_H
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define RIO_BUFSIZE 8192
typedef struct{
    int rio_fd;
    int rio_cnt;    //unread bytes in internal buf
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
}rio_t;

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, const void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp,void *usrbuf, size_t n);
#endif
