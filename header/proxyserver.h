#pragma once
#include"task.h"
#include "ThreadPool.hpp"
#include "mysql.h"
#include <vector>
#include <set>

#define LISTEN_PORT 20015		/* 代理服务器端口号 */
#define LISTEN_COUNT 40			/* ipv4监听数 */
#define THREAD_COUNT 40			/* ipv4请求处理线程数 */

class ProxyServer
{
public:
	ProxyServer();
	~ProxyServer();
	void proxyStartUp();

	void deleteTask(int taskSeq);			/* 删除指定任务 */
	std::vector<Task*> tasks;				/* 当前代理维护的任务列表 */

	/* 黑名单相关操作函数 */
	bool addDomain(std::string str);
	void deleteDomain(std::string str);
	std::set<std::string> getDomainList();
	bool addIp(std::string str);
	void deleteIp(std::string str);
	std::set<std::string> getIpList();
	bool addType(std::string str);
	void deleteType(std::string str);
	std::set<std::string> getTypeList();

private:
	SOCKET sockListen;					/* ipv4监听socket */
	SOCKADDR_IN sockAddrListen;			/* 监听的ipv4通信地址 */
	ThreadPool<Task>* pool;				/* 线程池，用于加入通信任务、获取当前任务队列 */

	/* 黑名单相关数据结构 */
	std::set<std::string> black_domain;	/* 访问域名黑名单 */
	std::set<std::string> black_ip;		/* 客户端ipv4地址黑名单 */
	std::set<std::string> black_type;	/* 传输文件类型黑名单 */
	std::mutex mtx_domain;
	std::mutex mtx_ip;
	std::mutex mtx_type;

	MYSQL mysql;

	bool initial();
	bool creatSocket();
	bool setBlackList();			/* 访问数据库获取初始黑名单 */
	void setDomainList();
	void setIpList();
	void setTypeList();
};
