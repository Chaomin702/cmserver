#include "epoll.h"

void Epoll::create(int flags){
	epfd_ = epoll_create(flags);
	if (eventsPtr_ != nullptr)
		delete[] eventsPtr_;
	eventsPtr_ = new epoll_event[MAXEVENTS + 1];
}

void Epoll::ctrl(int fd, long long data, uint32_t events, int op){
	struct epoll_event ev;
	ev.data.u64 = data;
	if (et_)
		ev.events = events | EPOLLET;
	else
		ev.events = events;
	epoll_ctl(epfd_, op, fd, &ev);
}

void Epoll::add(int fd, long long data, uint32_t event){
	ctrl(fd, data, event, EPOLL_CTL_ADD);
}

void Epoll::del(int fd, long long data, uint32_t event){
	ctrl(fd, data, event, EPOLL_CTL_DEL);
}

void Epoll::mod(int fd, long long data, uint32_t event){
	ctrl(fd, data, event, EPOLL_CTL_MOD);
}

int Epoll::wait(int millSecond){
	return epoll_wait(epfd_, eventsPtr_, MAXEVENTS + 1, millSecond);
}

