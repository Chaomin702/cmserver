#pragma once
#include "inetAddress.h"
#include <string>
#include "buffer.h"
class Socket {
public:
	explicit Socket(int sockfd) :sockfd_(sockfd), buf_(sockfd){}
	Socket(const Socket&) = delete;
	Socket& operator = (const Socket&) = delete;
	int fd()const { return sockfd_; }
	void bind(const InetAddress& addr);
	void listen();
	int accept(InetAddress *peeraddr);
	void setReuseAddr(bool on);
	std::string recv(size_t n);
	void send(const std::string& str);
	std::string readline();
private:
	const int sockfd_;
	Buffer buf_;
};