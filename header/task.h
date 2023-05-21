#pragma once
#include "managers.h"
#include <WS2tcpip.h>
#include <thread>

using namespace managers;

class Task {
public:
	Task(RuleManager* rule, CacheManager* cache, SOCKET sockConn, SOCKADDR_IN addr);
	~Task();
	void startup();
	bool isReady;					/* 两端信息是否已准备充分 */
	bool isEnd;						/* 任务是否已结束 */
	std::string source;				/* 客户端主机信息 */
	std::string dest;				/* 服务器主机信息 */
	std::string domain;				/* 访问的域名 */
	unsigned long long up_bytes;	/* 上传给服务器的应用层数据字节数 */
	unsigned long long down_bytes;	/* 从服务器下载的应用层数据字节数 */

private:
	bool preCheckHttp();
	bool preCheckHttps();
	void generateQString();
	void transferLoop();
	void getProtAndParseHead();
	void parseHttpsHead(std::string line);
	void parseHttpHead(std::string whole);
	char* getIpFromDomain();
	char* getIpv6(ADDRINFOA& hints, char* ipBuf);
	bool isClientLegal();
	bool isDomainLegal();
	bool isTypeLegal();

	SOCKET clientSocket;
	SOCKET serverSocket;
	ClientManager* clientManager;		/* 客户端管理器，负责与客户端的通信 */
	ServerManager* serverManager;		/* 服务器端管理器，负责与服务器端通信 */
	RuleManager* ruleManager;
	CacheManager* cacheManager;
	char* buffer;						/* 报文缓冲区 */
	int packLen;						/* 最近一次接收或发送的字节数 */
	std::string protocol;				/* 客户端使用的协议类型 */
	std::string httpVersion;			/* 客户端浏览器使用的http协议版本 */
	std::string url;
	bool isIpv4;
};
