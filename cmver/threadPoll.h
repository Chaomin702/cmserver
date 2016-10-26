#pragma once
#include "nonCopyable.h"
#include "mutexLock.h"
#include "condition.h"
#include "thread.h"
#include <queue>
#include <functional>
#include <memory>
#include <vector>

class ThreadPoll : private NonCopyable {
public:
	using Task = std::function<void()>;
	ThreadPoll(size_t queueSize, size_t pollSize);
	~ThreadPoll();
	void start();
	void stop();
	void addTask(const Task&);
	Task getTask();
private:
	void runInThread();
	mutable MutexLock mutex_;
	Condition empty_;
	Condition full_;
	size_t queueSize_;
	std::queue<Task> queue_;
	size_t pollSize_;
	std::vector<std::shared_ptr<Thread>> threads_;
	bool isStarted_;
};