#pragma once
#include <pthread.h>
#include <functional>
#include <iostream>
#include <functional>
#include "nonCopyable.h"

class Thread : NonCopyable {
public:
	using ThreadFunc = std::function<void()>;
	Thread();
	explicit Thread(const ThreadFunc &callback);
	~Thread();
	void setCallback(const ThreadFunc &callback);
	void start();
	void join();
private:
	static void* threadFunc(void *);
	pthread_t tid_;
	bool isStarted_;
	ThreadFunc callback_;
};