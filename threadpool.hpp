#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#define MAX_THREAD_SIZE 100			/* 线程库中的最大线程数 */

template<typename T>
class ThreadPool
{
public:
	ThreadPool(int count = 1);
	~ThreadPool();
	bool addTask(T* task);

private:
	condition_variable condition;
	queue<T*> task_queue;
	mutex mtx;
	vector<thread> work_threads;
	static void worker(void* arg);
	void run();
	bool end;
};

template<typename T>
ThreadPool<T>::ThreadPool(int count) :end(false)
{
	if (count <= 0 || count > MAX_THREAD_SIZE)
		throw exception();
	for (int i = 0; i < count; i++)
		work_threads.emplace_back(worker, this);
	cout << "Create " << count << " threads" << endl;
}


template<typename T>
ThreadPool<T>::~ThreadPool()
{
	end = true;
	condition.notify_all();
	for (thread& wt : work_threads)
		wt.join();
}

template<typename T>
bool ThreadPool<T>::addTask(T* task)
{
	if (!task)
		return false;
	unique_lock<mutex> lck(mtx);
	task_queue.push(task);
	condition.notify_one();
	return true;
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
		unique_lock<mutex> lck(mtx);
		condition.wait(lck, [this] {return !task_queue.empty() || end; });
		if (task_queue.empty())
			continue;
		else {
			T* task = task_queue.front();
			task_queue.pop();
			lck.unlock();
			task->startup();
			Task::count--;
			delete task;
		}
		cout << "-------------------- A task is over --------------------" << endl;
	}
}
