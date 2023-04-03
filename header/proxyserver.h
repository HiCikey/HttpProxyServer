#pragma once
#include"task.h"
#include "ThreadPool.hpp"
#include <vector>

#define LISTEN_PORT 20015		/* 代理服务器端口号 */
#define LISTEN_COUNT 40			/* ipv4监听数 */
#define THREAD_COUNT 40			/* ipv4请求处理线程数 */
#define LISTEN_COUNT_IPV6 20	/* ipv6监听数 */
#define THREAD_COUNT_IPV6 20	/* ipv6请求处理线程数 */


class ProxyServer
{
public:
	ProxyServer();
	~ProxyServer();

	SOCKET sockListen;					/* ipv4监听socket */
	SOCKADDR_IN sockAddrListen;			/* 监听的ipv4通信地址 */

	void proxyStartUp();
	queue<Task*> getTasks();
private:
	ThreadPool<Task>* pool;				/* 线程池，用于加入通信任务、获取当前任务队列 */

	bool initial();
	bool creatSocket();
};
