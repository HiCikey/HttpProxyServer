#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "global.h"
#include <WinSock2.h>
#include<WS2tcpip.h>
#include "mysql.h"
#include <set>
#pragma comment(lib,"ws2_32.lib")

namespace managers {
	/*
	* 缓存标签，可通过该结构体唯一标识一个缓存文件
	*/
	typedef struct CacheLabel {
		char* data;			/* 缓存文件内容指针 */
		u_int TTL;			/* 文件缓存的剩余时间（秒） */
		u_int visit_times;	/* 文件访问次数 */
		u_int size;			/* 文件大小（字节） */
	}*CacheLabelPtr;

	/*
	* 客户端通信管理类
	*/
	class ClientManager
	{
	public:
		ClientManager(SOCKET sockConn, SOCKADDR_IN addr);
		~ClientManager();
		bool recvFromClient(char* buf, int& packLen);		/* 接收客户端的请求报文 */
		bool sendToClient(char* buf, int size);				/* 转发服务器的回复报文给客户端 */

		ClientInfoPtr clientHost;		/* 客户端信息 */
	private:
		SOCKET sockClient;				/* 客户端通信socket */
	};

	/*
	* 服务器端通信管理类
	*/
	class ServerManager
	{
	public:
		ServerManager();
		~ServerManager();
		bool connectServer(SOCKET& sock);
		bool connectIpv6Server(SOCKET& sock);
		bool recvFromServer(char* buf, int& packLen);			/* 接收服务器的回复报文 */
		bool sendToServer(char* buf, int size);					/* 转发客户端的请求报文给服务器 */

		ServerInfoPtr serverHost;		/* 服务器端信息 */
	private:
		SOCKET sockServer;				/* 服务器端通信socket */
	};

	/*
	* http访问规则管理员类
	*/
	class RuleManager
	{
	public:
		RuleManager();
		~RuleManager();

		// 获取数据库中的黑名单
		bool setBlackList();

		// 获取内存中的黑名单副本
		std::set<std::string> getIpList();
		std::set<std::string> getDomainList();
		std::set<std::string> getTypeList();

		// 黑名单中增加一项
		bool addIp(std::string str);
		bool addDomain(std::string str);
		bool addType(std::string str);

		// 黑名单中删除一项
		void deleteIp(std::string str);
		void deleteDomain(std::string str);
		void deleteType(std::string str);

		// 查看黑名单中是否有指定内容
		bool checkIp(std::string str);
		bool checkDomain(std::string str);
		bool checkType(std::string str);
	private:
		MYSQL mysql;

		/* 黑名单相关数据结构 */
		std::set<std::string> black_domain;	/* 访问域名黑名单 */
		std::set<std::string> black_ip;		/* 客户端ipv4地址黑名单 */
		std::set<std::string> black_type;	/* 传输文件类型黑名单 */
		std::mutex mtx_domain;
		std::mutex mtx_ip;
		std::mutex mtx_type;

		void setDomainList();
		void setIpList();
		void setTypeList();
	};
}