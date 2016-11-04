#include "socketOps.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "dbg.h"
#include "errno.h"
typedef struct sockaddr SA;
using namespace sockets;
SA* sockaddr_cast(struct sockaddr_in *addr) {
	return static_cast<SA*>((void*)(addr));
}

const SA* sockaddr_cast(const struct sockaddr_in *addr) {
	return static_cast<SA*>((void*)(addr));
}

int sockets::createNonblocking() {
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		log_info("createNonblocking");
	setNonBlockAndCloseOnExec(sockfd);
	return sockfd;
}

void sockets::setNonBlockAndCloseOnExec(int sockfd){
	int flags = ::fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(sockfd, F_SETFL, flags);
	if (ret == -1)
		log_info("setNonBlockAndCloseOnExec");
	flags = ::fcntl(sockfd, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(sockfd, F_SETFD, flags);
	if (ret == -1)
		log_info("setNonBlockAndCloseOnExec");
}

int sockets::socket(){
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		log_info("createNonblocking");
	return sockfd;
}

int sockets::connect(int sockfd, const sockaddr_in & addr){
	return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

void sockets::bind(int sockfd, const struct sockaddr_in & addr){
	int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
	if (ret < 0)
		log_info("sockets::bind");
}

void sockets::listen(int sockfd){
	int ret = ::listen(sockfd, SOMAXCONN);
	if (ret < 0)
		log_info("sockets::listen");
}

int sockets::accept(int sockfd, sockaddr_in * addr){
	socklen_t addrlen = sizeof *addr;
	int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
	if (connfd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			log_info("sockets::accept");
	}
	return connfd;
}

void sockets::close(int sockfd){
	if (::close(sockfd) < 0)
		log_info("sockets::close");
}

std::string sockets::readn(int fd, size_t n){
	char buf[RIO_BUFSIZE];
	ssize_t r = ::rio_readn(fd, buf, n);
	if (r < 0)
		log_info("sockets::readn");
	return std::string(buf, r);
}

ssize_t sockets::writen(int fd, const std::string & str){
	const char* p = str.data();
	return ::rio_writen(fd, p, str.size());
}

sigfunc* signalRaw(int signo, sigfunc *func) {
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM)
		act.sa_flags |= SA_RESTART;
	if (sigaction(signo, &act, &oact) < 0)
		return (SIG_ERR);
	return oact.sa_handler;
}
sigfunc* sockets::signal(int signo, sigfunc *func) {
	sigfunc *sf;
	if ((sf = signalRaw(signo, func)) == SIG_ERR)
		log_err("signal function error");
	return sf;
}
