#include "socket.h"
#include <sys/socket.h>
#include "socketOps.h"
#include "dbg.h"
void Socket::bind(const InetAddress & addr){
	sockets::bind(sockfd_, addr.getSocketAddr());
}

void Socket::listen(){
	sockets::listen(sockfd_);
}

int Socket::accept(InetAddress * peeraddr){
	struct sockaddr_in addr;
	bzero(&addr,sizeof addr);
	int connfd = sockets::accept(sockfd_, &addr);
	if (connfd >= 0)
		peeraddr->setSockAddrInet(addr);
	return connfd;
}

void Socket::setReuseAddr(bool on){
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

std::string Socket::recv(size_t n){
	return buf_.readn(n);
}

void Socket::send(const std::string & str){
	buf_.writen(str);
}

std::string Socket::readline(){
	return buf_.readline();
}

