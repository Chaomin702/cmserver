#include "buffer.h"

ssize_t Buffer::readn(void * usrbuf, size_t n)
{
	return rio_readn(data.rio_fd, usrbuf, n);
}

ssize_t Buffer::writen(const void * usrbuf, size_t n){
	return rio_writen(data.rio_fd, usrbuf, n);
}

ssize_t Buffer::readlineb(void * usrbuf, size_t maxlen){
	return rio_readlineb(&data, usrbuf, maxlen);
}

ssize_t Buffer::readnb(void * usrbuf, size_t n){
	return rio_readnb(&data, usrbuf, n);
}

std::string Buffer::readn(size_t n){
	char buf[RIO_BUFSIZE];
	int r = readn(buf, n);
	if (r < 0)
		errorMsg("Buffer::readn");
	return std::string(buf, r);
}

ssize_t Buffer::writen(const std::string & str){
	const char *s = str.data();
	return writen(s, str.size());
}

std::string Buffer::readline(){
	char buf[RIO_BUFSIZE];
	int r = readlineb(buf, RIO_BUFSIZE);
	if (r < 0)
		errorMsg("Buffer::readline");
	return std::string(buf, r);
}


