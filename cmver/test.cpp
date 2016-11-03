#include <iostream>
#include <memory>
#include <functional>
#include "socket.h"
#include "socketOps.h"
#include "epoll.h"
#include "threadPoll.h"
#include "httpParser.h"

void func(int fd){
	std::cout << "fd: " << fd << "\n";
	char buf[RIO_BUFSIZE];
	int r =::read(fd, buf, RIO_BUFSIZE);
	std::cout << buf << std::endl;
	::write(fd, "haha\n", 5);
	::close(fd);
}

int main(void) {
	ThreadPoll tp(100, 10);
	tp.start();
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
				int fd = ev.data.fd;
				cm_http::handleHttpRequest hhp = std::make_shared<cm_http::httpRequest>(fd);
				tp.addTask(std::bind(cm_http::doRequest, hhp));
			}
			else {
				errorMsg("epoll error");
			}
		}
	}
	return 0;
}