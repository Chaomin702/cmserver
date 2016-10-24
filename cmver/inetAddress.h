#pragma once
#include <netinet/in.h>
#include <string>
class InetAddress{
public:
	explicit InetAddress(uint16_t port);
	InetAddress(const std::string& ip, uint16_t port);
	const struct sockaddr_in& getSocketAddr()const { return addr_; }
	void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }
private:
	struct sockaddr_in addr_;
};
