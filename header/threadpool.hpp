#pragma once
#include <iostream>
#include <stdio.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#define MAX_THREAD_SIZE 100			/* 线程库中的最大线程数 */

template<typename T>
class ThreadPool
{
public:
	ThreadPool(int count = 1);
	~ThreadPool();
	void addTask(T* task);

private:
	std::condition_variable condition;
	std::queue<T*> task_queue;
	std::mutex mtx;
	std::vector<std::thread> work_threads;
	static void worker(void* arg);
	void run();
	bool end;
};

template<typename T>
ThreadPool<T>::ThreadPool(int count) :end(false)
{
	if (count <= 0 || count > MAX_THREAD_SIZE)
		throw std::exception();
	for (int i = 0; i < count; i++)
		work_threads.emplace_back(worker, this);
	printf("Create %d threads\n", count);
}


template<typename T>
ThreadPool<T>::~ThreadPool()
{
	end = true;
	condition.notify_all();
	for (std::thread& wt : work_threads)
		wt.join();
}

template<typename T>
void ThreadPool<T>::addTask(T* task)
{
	std::unique_lock<std::mutex> lck(mtx);
	task_queue.push(task);
	condition.notify_one();
}

template<typename T>
inline void ThreadPool<T>::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	pool->run();
}

template<typename T>
void ThreadPool<T>::run()
{
	while (!end)
	{
		std::unique_lock<std::mutex> lck(mtx);
		condition.wait(lck, [this] {return !task_queue.empty() || end; });
		if (task_queue.empty())
			continue;
		else {
			T* task = task_queue.front();
			task_queue.pop();
			lck.unlock();
			task->startup();
			task->isEnd = true;
			printf("\n\n");
			Task::count--;
		}
	}
}
