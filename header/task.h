#pragma once
#include "managers.h"
#include <WS2tcpip.h>
#include <thread>

using namespace std;
using namespace managers;

class Task {
public:
	Task(SOCKET sockConn, SOCKADDR_IN addr);
	~Task();
	void startup();
	static int count;			//hack 调式完成后删除该变量及其所有引用
private:
	void transferLoop();
	void getProtAndParseHead();
	void parseHttpsHead(string line);
	void parseHttpHead(string whole);
	char* getIpFromDomain(string domain);
	char* getIpv6(string domain, ADDRINFOA& hints, char* ipBuf);

	SOCKET clientSocket;
	SOCKET serverSocket;
	ClientManager* clientManager;			/* 客户端管理器，负责与客户端的通信 */
	ServerManager* serverManager;			/* 服务器端管理器，负责与服务器端通信 */
	char* buffer;						/* 报文缓冲区 */
	int packLen;						/* 最近一次接收或发送的字节数 */
	string protocol;					/* 客户端使用的协议类型 */
	string httpVersion;					/* 客户端浏览器使用的http协议版本 */
	bool isIpv4;						// hack 删除否？
	unsigned long long up_bytes;		/* 上传给服务器的应用层数据字节数 */
	unsigned long long down_bytes;		/* 从服务器下载的应用层数据字节数 */
};
