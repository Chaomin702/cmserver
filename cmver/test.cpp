#include <iostream>
#include "socket.h"
#include "socketOps.h"

int main(void) {

	Socket s(sockets::socket());
	s.setReuseAddr(true);
	InetAddress addr(3000);
	s.bind(addr);
	s.listen();
	InetAddress cliaddr(0);
	while (true) {
	}
	return 0;
}