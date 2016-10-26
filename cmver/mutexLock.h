#pragma once
#include <pthread.h>
#include <assert.h>
#include "nonCopyable.h"
class MutexGuard;
class MutexLock :NonCopyable{
public:
	friend class MutexGuard;
	MutexLock() :islocked_(false) {
		pthread_mutex_init(&mutex_, NULL);
	}
	~MutexLock() {
		assert(islocked_ == false);
		pthread_mutex_destroy(&mutex_);
	}
	pthread_mutex_t* getPthreadMutex() { return &mutex_; }
	bool islocked() const{ return islocked_; }
private:
	void lock() {
		pthread_mutex_lock(&mutex_);
		islocked_ = true;
	}
	void unlock() {
		pthread_mutex_unlock(&mutex_);
		islocked_ = false;
	}
	pthread_mutex_t mutex_;
	bool islocked_;
};

class MutexGuard {
public:
	MutexGuard(MutexLock& mutex) :mutex_(mutex) {
		mutex_.lock();
	}
	~MutexGuard() {
		mutex_.unlock();
	}
private:
	MutexLock& mutex_;
};
#define MutexGuard(m) "ERROR"