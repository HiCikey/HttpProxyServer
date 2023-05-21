#pragma once
#include"task.h"
#include "ThreadPool.hpp"
#include <vector>

#define LISTEN_PORT 20015		/* 代理服务器端口号 */
#define LISTEN_COUNT 25			/* ipv4监听数 */
#define THREAD_COUNT 100		/* ipv4请求处理线程数 */

class ProxyServer
{
public:
	ProxyServer();
	~ProxyServer();

	void proxyStartUp();
	void deleteTask(int taskSeq);			/* 删除指定任务 */
	std::vector<Task*> tasks;				/* 当前代理维护的任务列表 */

	RuleManager* ruleManager;
private:
	SOCKET sockListen;					/* ipv4监听socket */
	SOCKADDR_IN sockAddrListen;			/* 监听的ipv4通信地址 */
	ThreadPool<Task>* pool;				/* 线程池，用于加入通信任务、获取当前任务队列 */
	CacheManager* cacheManager;

	bool initial();
	bool creatSocket();
};
