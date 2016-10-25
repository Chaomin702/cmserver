#pragma once
extern "C" {
#include "rio.h"
}
#include <string>
#include "dbg.h"
class Buffer {
public:
	explicit Buffer(int fd) {
		rio_readinitb(&data, fd);
	}
	ssize_t readn(void *usrbuf, size_t n);
	ssize_t writen(const void *usrbuf, size_t n);
	ssize_t readlineb(void *usrbuf, size_t maxlen);
	ssize_t readnb(void *usrbuf, size_t n);

	std::string readn(size_t n);
	ssize_t writen(const std::string& str);
	std::string readline();

private:
	rio_t data;
};