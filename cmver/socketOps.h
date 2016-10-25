#pragma once
#include <netinet/in.h>
#include <string>
extern "C" {
#include "rio.h"
}
namespace sockets {
	int createNonblocking();
	void setNonBlockAndCloseOnExec(int sockfd);
	int socket();
	int connect(int sockfd, const struct sockaddr_in& addr);
	void bind(int sockfd, const struct sockaddr_in& addr);
	void listen(int sockfd);
	int accept(int sockfd, struct sockaddr_in *addr);
	void close(int sockfd);
	std::string readn(int fd, size_t n);
	ssize_t writen(int fd, const std::string& str);
}