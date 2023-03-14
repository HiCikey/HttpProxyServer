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
	SOCKET sockListenIpv6;				/* ipv6监听socket */
	SOCKADDR_IN sockAddrListen;			/* 监听的ipv4通信地址 */
	SOCKADDR_IN6 sockAddrListenIpv6;	/* 监听的ipv6通信地址 */

	void proxyStartUp();
private:
	bool initial();
	bool creatSocket();
};
