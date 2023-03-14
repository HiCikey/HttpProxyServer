#pragma once
#include "manager.h"
#include <WS2tcpip.h>
#include <thread>

using namespace std;
using namespace manager;

class Task {
public:
	Task(SOCKET sockConn, SOCKADDR_IN addr);
	Task(SOCKET sockConnIpv6, SOCKADDR_IN6 addrIpv6);
	~Task();
	void startup();
	static int count;			//hack 调式完成后删除该变量及其所有引用
private:
	void getProtAndParseHead();
	void parseHttpsHead(string line);
	void parseHttpHead(string whole);
	char* getIpFromDomain(string domain);
	char* getIpv6(string domain, ADDRINFOA& hints, char* ipBuf);

	ClientManager* cliManager;			/* 客户端管理器，负责与客户端的通信 */
	ServerManager* serManager;			/* 服务器端管理器，负责与服务器端通信 */
	char* buffer;						/* 报文缓冲区 */
	int packLen;
	string protocol;					/* 客户端使用的协议类型 */
	string httpVersion;					/* 客户端浏览器使用的http协议版本 */
	bool isIpv4;
};