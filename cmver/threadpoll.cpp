#include "threadPoll.h"

ThreadPoll::ThreadPoll(size_t queueSize, size_t pollSize)
	: empty_(mutex_)
	, full_(mutex_)
	, queueSize_(queueSize)
	, pollSize_(pollSize)
	, isStarted_(false){}
ThreadPoll::~ThreadPoll(){
	if (isStarted_)
		stop();
}
void ThreadPoll::start(){
	if (isStarted_)
		return;
	isStarted_ = true;
	for (size_t i = 0; i < pollSize_; ++i){
		threads_.push_back(std::make_shared<Thread>(std::bind(&ThreadPoll::runInThread, this)));
	}
	for (auto &i : threads_){
		i->start();
	}
}

void ThreadPoll::addTask(const Task& t){
	MutexGuard lock(mutex_);
	while (queue_.size() == queueSize_){
		full_.wait();
	}
	queue_.push(t);
	empty_.notify();
}

ThreadPoll::Task ThreadPoll::getTask(){
	MutexGuard lock(mutex_);
	while (queue_.empty() && isStarted_){
		empty_.wait();
	}
	Task t;
	if (!queue_.empty()) {
		t = queue_.front();
		queue_.pop();
		full_.notify();
	 }
	return t;
}


void ThreadPoll::runInThread(){
	while (isStarted_){
		Task t = getTask();
		if(t)
			t();
	}
}

void ThreadPoll::stop(){
	if (isStarted_ == false)
		return;
	{
		MutexGuard lock(mutex_);
		isStarted_ = false;
		empty_.notifyAll();
	}
	for (auto &i : threads_){
		i->join();
	}
	while (!queue_.empty())
		queue_.pop();
}