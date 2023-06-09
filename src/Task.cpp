﻿#include "task.h"

Task::Task(RuleManager* rule, CacheManager* cache, SOCKET sockConn, SOCKADDR_IN addr)
	: ruleManager(rule)
	, cacheManager(cache)
	, isIpv4(true)
	, isReady(false)
	, isEnd(false)
{
	clientManager = new ClientManager(sockConn, addr);
	serverManager = new ServerManager();
	buffer = new char[BUFFER_SIZE];
	clientSocket = sockConn;
	serverSocket = INVALID_SOCKET;
	source = "";
	dest = "";
	domain = "";
	url = "";
	up_bytes = 0;
	down_bytes = 0;
	packLen = -1;
	protocol = "";
	httpVersion = "1.1";
}

Task::~Task()
{
	delete buffer;
	delete clientManager;
	delete serverManager;
}

void Task::startup()
{
	if (!isClientLegal() || !clientManager->recvFromClient(buffer, packLen)) return;
	getProtAndParseHead();
	if (!isDomainLegal()) return;

	if ((isIpv4 && !serverManager->connectServer(serverSocket)) || (!isIpv4 && !serverManager->connectIpv6Server(serverSocket))) return;
	printf("TCP tunnel has been established   %s:%d | [%s]%s:%u\n", clientManager->clientHost->addr, clientManager->clientHost->port, serverManager->serverHost->addr, serverManager->serverHost->domain.c_str(), serverManager->serverHost->port);

	if (protocol == "HTTP" && !preCheckHttp())
		return;
	else if (protocol == "HTTPS" && !preCheckHttps())
		return;

	serverManager->sendToServer(buffer, packLen);
	transferLoop();
}


bool Task::preCheckHttp()
{
	if (!isTypeLegal()) return false;
	std::pair<char*, int> res = cacheManager->serchLable(url);
	if (res.second != -1) {
		clientManager->sendToClient(res.first, res.second);
		if (!clientManager->recvFromClient(buffer, packLen))
			return false;
	}
	generateQString();
	isReady = true;
	return true;
}

bool Task::preCheckHttps()
{
	generateQString();
	isReady = true;

	/* 若为https协议，回复客户端已经建立和服务器之间的tcp连接，并接收客户端的真正https请求报文 */
	std::string res = "HTTP/" + httpVersion + " 200 Connection established\r\n\r\n";
	clientManager->sendToClient(const_cast<char*>(res.c_str()), (int)res.size());
	if (!clientManager->recvFromClient(buffer, packLen))
		return false;
	return true;
}

/*  检查客户端IP是否合法*/
bool Task::isClientLegal()
{
	std::string clientIp = std::string(clientManager->clientHost->addr);
	return !ruleManager->checkIp(clientIp);
}


/* 检查要访问的域名是否合法 */
inline bool Task::isDomainLegal()
{
	return !ruleManager->checkDomain(domain);
}


/* 检查传输文件类型是否合法 */
bool Task::isTypeLegal()
{
	std::string package = std::string(buffer);
	std::string firstLine = package.substr(0, package.find("\r\n"));
	int dotLoc = -1, leanLine = -1;
	std::string type = firstLine.substr(firstLine.find(" ") + 1);	/* 提取出url部分存在type中 */
	type = type.substr(0, type.rfind(" HTTP"));
	url = domain + type;
	dotLoc = type.rfind(".");		/* 判断该报文是否指定接受的文件后缀 */
	if (dotLoc == -1)
		return true;
	type = type.substr(dotLoc);
	return !ruleManager->checkType(type);
}

inline void Task::generateQString()
{
	source = std::string(clientManager->clientHost->addr) + ":" + std::to_string(clientManager->clientHost->port);
	dest = std::string(serverManager->serverHost->addr) + ":" + std::to_string(serverManager->serverHost->port);
}


/*
* 循环转发客户端和目的服务器之间的通信报文
* 直到一方断开连接或转发失败
*/
void Task::transferLoop()
{
	fd_set read_set = { 0 };
	/* 转发报文循环 */
	while (true)
	{
		FD_ZERO(&read_set);
		FD_SET(clientSocket, &read_set);
		FD_SET(serverSocket, &read_set);
		select(FD_SETSIZE, &read_set, NULL, NULL, NULL);

		// 服务器--->客户端有信息要转发
		if (FD_ISSET(serverSocket, &read_set)) {
			if (!serverManager->recvFromServer(buffer, packLen))
				break;
			if (protocol == "HTTP")
				cacheManager->addLable(url, buffer, packLen);
			down_bytes += packLen;
			clientManager->sendToClient(buffer, packLen);
		}
		// 客户端--->服务器有信息要转发
		if (FD_ISSET(clientSocket, &read_set)) {
			if (!clientManager->recvFromClient(buffer, packLen))
				break;
			if (protocol == "HTTP") {
				if (!isTypeLegal())
					continue;
				std::pair<char*, int> res = cacheManager->serchLable(url);
				if (res.second != -1) {
					clientManager->sendToClient(res.first, res.second);
					continue;
				}
			}
			up_bytes += packLen;
			serverManager->sendToServer(buffer, packLen);
		}
	}
}


/*
* 根据客户端第一个报文类型分析客户端使用的协议，并调用对应内部方法解析报文头部
* CONNECT报文：https协议
* 其他类型报文：http协议
*/
void Task::getProtAndParseHead()
{
	std::string bufStr = std::string(buffer);
	std::string line;			/* 报文内容的第一行 */
	std::string type;			/* 报文类型 */

	line = bufStr.substr(0, bufStr.find("\r\n"));
	type = line.substr(0, line.find(" "));
	if (type == "CONNECT") {
		protocol = "HTTPS";
		parseHttpsHead(line);
	}
	else {
		protocol = "HTTP";
		parseHttpHead(bufStr);
	}
}


/*
* 解析https协议CONNECT报文头部
* 获取目的web服务器的域名、ip、端口
* CONNECT报文第一行内容格式：CONNECT [domain]:443 HTTP/[version]
*/
void Task::parseHttpsHead(std::string line)
{
	size_t hostLen;		/* 主机长度 */
	std::string host;	/* host(domain:port) */

	hostLen = line.find(" HTTP/") - 8;
	host = line.substr(8, hostLen);
	domain = host.substr(0, hostLen - 4);
	httpVersion = line.substr(hostLen + 8 + 6);

	serverManager->serverHost->port = 443;		/* https协议服务器端口默认为443 */
	serverManager->serverHost->domain = domain;
	serverManager->serverHost->addr = getIpFromDomain();
}


/*
* 解析http协议 GET或POST 报文头部
* 获取目的web服务器的域名、ip、端口
*/
void Task::parseHttpHead(std::string whole)
{
	std::string line;					/* 保存每行内容直到解析出目的主机 */
	int pos = whole.find("\r\n");		/* 用于记录当前行的结束位置 */
	int size = whole.size();			/* 记录整个请求报文长度 */
	bool end = false;
	int port = 80;						/* http协议服务器端口默认为80 */

	while (!end && pos < size - 2)
	{
		whole = whole.substr(pos + 2);		/* 去掉第一行内容，第一行包含url的后缀部分，可用于禁用传输文件类型 */
		pos = whole.find("\r\n");
		line = whole.substr(0, pos);		/* 当前whole的第一行内容 */
		if (line.find("Host:") == 0)		/* 判断此行内容是否是目的host */
		{
			line = line.substr(6);
			int p = static_cast<int>(line.find(":"));
			if (p != -1) {
				domain = line.substr(0, p);
				port = stoi(line.substr(p + 1));
			}
			else
				domain = line;
			end = true;
		}
	}
	if (end) {
		serverManager->serverHost->domain = domain;		/* 保存解析出的目的主机信息 */
		serverManager->serverHost->port = port;
		serverManager->serverHost->addr = getIpFromDomain();
	}
}


/* 通过目标域名获取目的服务器ip地址 */
char* Task::getIpFromDomain()
{
	ADDRINFOA hints;		/* 指出调用方支持的套接字类型 */
	PADDRINFOA res;			/* 链表结构，保存目的主机的所有IP信息 */
	char* ipBuf;			/* 获得的IP地址 */

	ZeroMemory(&hints, sizeof(ADDRINFOA));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(domain.c_str(), NULL, &hints, &res) != 0) {
		ipBuf = new char[ADDRLEN_IPV6];
		return getIpv6(hints, ipBuf);		/* 该域名未解析出ipv4地址，则尝试获得域名的ipv6地址 */
	}

	PSOCKADDR_IN addr;
	ipBuf = new char[ADDRLEN_IPV4];
	addr = (PSOCKADDR_IN)res->ai_addr;				/* 拥有多个ip地址时只保存第一个 */
	inet_ntop(AF_INET, &addr->sin_addr, ipBuf, ADDRLEN_IPV4);
	return ipBuf;
}


/* 尝试解析出域名对应的ipv6地址，存入ipBuf中 */
char* Task::getIpv6(ADDRINFOA& hints, char* ipBuf)
{
	PADDRINFOA res;
	hints.ai_family = AF_INET6;
	if (getaddrinfo(domain.c_str(), NULL, &hints, &res) != 0) {
		delete[] ipBuf;
		return nullptr;
	}

	PSOCKADDR_IN6 addr6;
	addr6 = (PSOCKADDR_IN6)res->ai_addr;
	inet_ntop(AF_INET6, &addr6->sin6_addr, ipBuf, ADDRLEN_IPV6);
	this->isIpv4 = false;
	return ipBuf;
}
