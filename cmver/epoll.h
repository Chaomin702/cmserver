#pragma once
#include <sys/epoll.h>
#include <assert.h>
const int MAXEVENTS = 1024;
class Epoll {
public:
	Epoll(bool et):epfd_(-1), eventsPtr_(nullptr), et_(et) {}
	void create(int flags);
	void ctrl(int fd, long long data, uint32_t events, int op);
	void add(int fd, long long data, uint32_t event);
	void del(int fd, long long data, uint32_t event);
	void mod(int fd, long long data, uint32_t enent);
	int wait(int millSecond);
	struct epoll_event& get(int i) { assert(eventsPtr_ != nullptr); return eventsPtr_[i]; }
private:
	int epfd_;
	struct epoll_event *eventsPtr_;
	bool et_;	//edge trigger?
};