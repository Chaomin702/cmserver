#include "thread.h"

Thread::Thread():tid_(-1),isStarted_(false){}

Thread::Thread(const ThreadFunc & callback):tid_(-1),isStarted_(false),callback_(callback){}

Thread::~Thread(){
	if (isStarted_)
		pthread_detach(tid_);
}

void Thread::setCallback(const ThreadFunc & callback){
	callback_ = callback;
}

void Thread::start(){
	pthread_create(&tid_, NULL, threadFunc, this);
}

void Thread::join(){
	pthread_join(tid_, NULL);
}

void * Thread::threadFunc(void *arg){
	Thread* pt = static_cast<Thread*>(arg);
	pt->callback_();
}


