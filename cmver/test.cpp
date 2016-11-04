#include <iostream>
#include <memory>
#include <functional>
#include "socket.h"
#include "socketOps.h"
#include "epoll.h"
#include "threadPoll.h"
#include "httpParser.h"

void func(int signo){
	std::cout << signo << " PIPE signal ignore" << std::endl;
}

int main(void) {
	sockets::signal(SIGPIPE, func);
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
			log_info("epoll:wait error");
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
				log_info("epoll error");
			}
		}
	}
	return 0;
}