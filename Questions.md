# Epoll

- level trigger 模式即使完全读完数据，下次还是会触发读

# Pthread

> If **pthread_mutex_destroy**() is called on a mutex that is locked by another thread, the request fails with an **EBUSY** error. If the calling thread has the mutex locked, any other threads waiting for the mutex using a call to **pthread_mutex_lock**() at the time of the call to **pthread_mutex_destroy**() fails with the **EDESTROYED** error.
>
> Mutex initialization using the **PTHREAD_MUTEX_INITIALIZER** does not immediately initialize the mutex. Instead, on first use, **pthread_mutex_lock**() or **pthread_mutex_trylock**() branches into a slow path and causes the initialization of the mutex. Because a mutex is not just a simple memory object and requires that some resources be allocated by the system, an attempt to call **pthread_mutex_destroy**() or **pthread_mutex_unlock**() on a mutex that has statically initialized using **PTHREAD_MUTEX_INITIALER** and was not yet locked causes an **EINVAL** error.
>
> Every mutex must eventually be destroyed with **pthread_mutex_destroy**(). The machine eventually detects the error if a mutex is not destroyed, but the storage is deallocated or corrupted. The machine then creates LIC log synchronization entries that indicate the failure to help debug the problem. Large numbers of these entries can affect system performance and hinder debug capabilities for other system problems. Always use **pthread_mutex_destroy**() before freeing mutex storage to prevent these debug LIC log entries.
>
> **Note:** Once a mutex is created, it cannot be validly copied or moved to a new location.

**pthread_mutex_destroy**必须调用

+ C++类成员变量初始化顺序，成员变量可为引用类型？
+ 消费者-生产者模型中，notify与notifyAll