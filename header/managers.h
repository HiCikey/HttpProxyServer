#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "global.h"
#include <WinSock2.h>
#include<WS2tcpip.h>
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
		ClientManager(SOCKET sockConnIpv6, SOCKADDR_IN6 addrIpv6);
		~ClientManager();
		void recvFromClient(char* buf, int& packLen);		/* 接收客户端的请求报文 */
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
		bool connectServer();
		bool connectIpv6Server();
		void recvFromServer(char* buf, int& packLen);			/* 接收服务器的回复报文 */
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
		static RuleManager* getInstance() { return nullptr; }		//hack RuleManager修改，继续使用单例模式？
	private:
		RuleManager();
		~RuleManager();
		//static RuleManager* ruleManager;	/* 类单例，保证整个系统只有一个规则管理员 */
	};
}

//RuleManager* RuleManager::ruleManager = new RuleManager();