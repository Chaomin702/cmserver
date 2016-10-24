#pragma once
#include <netinet/in.h>
namespace sockets {
	int createNonblocking();
	int socket();
	int connect(int sockfd, const struct sockaddr_in& addr);
	void bind(int sockfd, const struct sockaddr_in& addr);
	void listen(int sockfd);
	int accept(int sockfd, struct sockaddr_in *addr);
	void close(int sockfd);
}