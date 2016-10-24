#include "socketOps.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "dbg.h"
#include "errno.h"
typedef struct sockaddr SA;

SA* sockaddr_cast(struct sockaddr_in *addr) {
	return static_cast<SA*>((void*)(addr));
}

const SA* sockaddr_cast(const struct sockaddr_in *addr) {
	return static_cast<SA*>((void*)(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
	int flags = ::fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(sockfd, F_SETFL, flags);
	if (ret == -1)
		errorMsg("setNonBlockAndCloseOnExec");
	flags = ::fcntl(sockfd, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(sockfd, F_SETFD, flags);
	if (ret == -1)
		errorMsg("setNonBlockAndCloseOnExec");
}

int sockets::createNonblocking() {
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		errorMsg("createNonblocking");
	setNonBlockAndCloseOnExec(sockfd);
	return sockfd;
}

int sockets::socket(){
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		errorMsg("createNonblocking");
	return sockfd;
}

int sockets::connect(int sockfd, const sockaddr_in & addr){
	return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

void sockets::bind(int sockfd, const struct sockaddr_in & addr){
	int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
	if (ret < 0)
		errorMsg("sockets::bind");
}

void sockets::listen(int sockfd){
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0)
		errorMsg("sockets::listen");
}

int sockets::accept(int sockfd, sockaddr_in * addr){
	socklen_t addrlen = sizeof *addr;
	int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
	if (connfd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			errorMsg("sockets::accept");
	}
	return connfd;
}

void sockets::close(int sockfd){
	if (::close(sockfd) < 0)
		errorMsg("sockets::close");
}



