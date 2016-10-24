#pragma once
#include "inetAddress.h"
class Socket {
public:
	explicit Socket(int sockfd) :sockfd_(sockfd) {}
	Socket(const Socket&) = delete;
	Socket& operator = (const Socket&) = delete;

	int fd()const { return sockfd_; }
	void bind(const InetAddress& addr);
	void listen();
	int accept(InetAddress *peeraddr);
	void setReuseAddr(bool on);
private:
	const int sockfd_;
};