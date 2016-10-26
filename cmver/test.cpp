#include <iostream>
#include "socket.h"
#include "socketOps.h"
#include "epoll.h"
extern "C" {
#include "rio.h"
}
int main(void) {
	char buf[RIO_BUFSIZE];
	Socket s(sockets::socket());
	s.setReuseAddr(true);
	InetAddress addr(3000);
	s.bind(addr);
	s.listen();
	InetAddress cliaddr(0);
	sockets::setNonBlockAndCloseOnExec(s.fd());
	Epoll poll(true);
	poll.create(100);
	poll.add(s.fd(), s.fd(), EPOLLIN);
	while (true) {
		int n = poll.wait(-1);
		if (n == -1)
			errorMsg("epoll:wait error");
		for (int i = 0; i < n; ++i) {
			auto &ev = poll.get(i);
			if (ev.data.fd == s.fd()) {
				int connfd = s.accept(&cliaddr);
				sockets::setNonBlockAndCloseOnExec(connfd);
				poll.add(connfd, connfd, EPOLLIN);
				std::cout << "new connect " << connfd << std::endl;
			}
			else if (ev.events & EPOLLIN) {
				int r = ::read(ev.data.fd, buf, RIO_BUFSIZE);
				std::cout <<"read size :" << r << std::endl;
				::write(ev.data.fd, "haha\n", 5);
				::close(ev.data.fd);
			}
			else {
				errorMsg("epoll error");
			}
		}
	}
	return 0;
}