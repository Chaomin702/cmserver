#include <iostream>
#include "socket.h"
#include "socketOps.h"
#include "epoll.h"
extern "C" {
#include "rio.h"
}
int main(void) {

	Socket s(sockets::socket());
	s.setReuseAddr(true);
	InetAddress addr(3000);
	s.bind(addr);
	s.listen();
	InetAddress cliaddr(0);
	while (true) {
		int infd = s.accept(&cliaddr);
		//sockets::setNonBlockAndCloseOnExec(infd);
		Socket ss(infd);
		std::string rv = ss.readline();
		std::cout << rv;
		std::cout << infd << "\n";
		ss.send("Thanks!\n");
		sockets::close(infd);
	}
	return 0;
}