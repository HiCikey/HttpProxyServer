#include "task.h"

Task::Task(SOCKET sockConn, SOCKADDR_IN addr) :isIpv4(true)
{
	clientManager = new ClientManager(sockConn, addr);
	serverManager = new ServerManager();
	buffer = new char[BUFFER_SIZE];
	clientSocket = sockConn;
	serverSocket = INVALID_SOCKET;
	up_bytes = 0;
	down_bytes = 0;
	packLen = -1;
	protocol = "";
	httpVersion = "1.1";
	cout << "\n!!!!!!!!!!!!!!! There are " << count << " tasks now !!!!!!!!!!!!!!!\n\n";
}

Task::~Task()
{
	delete buffer;
	delete clientManager;
	delete serverManager;
	cout << "\n!!!!!!!!!!!!!!! There are " << count << " tasks now !!!!!!!!!!!!!!!\n\n";
}


void Task::startup()
{
	/* 接收客户端的第一个报文，解析使用的传输协议，并与目标服务器建立tcp连接 */
	if (!clientManager->recvFromClient(buffer, packLen)) return;
	if (packLen <= 0) return;
	getProtAndParseHead();
	if ((isIpv4 && !serverManager->connectServer(serverSocket)) || (!isIpv4 && !serverManager->connectIpv6Server(serverSocket)))
		return;
	printf("TCP tunnel has been established   %s:%d | [%s]%s:%u\n", clientManager->clientHost->addr, clientManager->clientHost->port, serverManager->serverHost->addr, serverManager->serverHost->domain.c_str(), serverManager->serverHost->port);

	/* 若为https协议，回复客户端已经建立和服务器之间的tcp连接，并接收客户端的真正https请求报文 */
	if (protocol == "HTTPS") {
		string res = "HTTP/" + httpVersion + " 200 Connection established\r\n\r\n";
		clientManager->sendToClient(const_cast<char*>(res.c_str()), (int)res.size());
		if (!clientManager->recvFromClient(buffer, packLen)) return;
	}
	serverManager->sendToServer(buffer, packLen);
	transferLoop();
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

		if (FD_ISSET(serverSocket, &read_set)){
			if (!serverManager->recvFromServer(buffer, packLen))
				break;
			clientManager->sendToClient(buffer, packLen);
		}
		if (FD_ISSET(clientSocket, &read_set)){
			if (!clientManager->recvFromClient(buffer, packLen))
				break;
			serverManager->sendToServer(buffer, packLen);
		}
	}
	cout << endl;
}


/*
* 根据客户端第一个报文类型分析客户端使用的协议，并调用对应内部方法解析报文头部
* CONNECT报文：https协议
* 其他类型报文：http协议
*/
void Task::getProtAndParseHead()
{
	string bufStr = string(buffer);
	string line;			/* 报文内容的第一行 */
	string type;			/* 报文类型 */

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
void Task::parseHttpsHead(string line)
{
	size_t hostLen;		/* 主机长度 */
	string host;		/* host(domain:port) */
	string domain;		/* 域名 */

	hostLen = line.find(" HTTP/") - 8;
	host = line.substr(8, hostLen);
	domain = host.substr(0, hostLen - 4);
	httpVersion = line.substr(hostLen + 8 + 6);

	serverManager->serverHost->port = 443;		/* https协议服务器端口默认为443 */
	serverManager->serverHost->domain = domain;
	serverManager->serverHost->addr = getIpFromDomain(domain);
}


/*
* 解析http协议 GET或POST 报文头部
* 获取目的web服务器的域名、ip、端口
*/
void Task::parseHttpHead(string whole)
{
	string line;						/* 保存每行内容直到解析出目的主机 */
	int pos = whole.find("\r\n");		/* 用于记录当前行的结束位置 */
	int size = whole.size();			/* 记录整个请求报文长度 */
	bool end = false;
	string domain;
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
		serverManager->serverHost->addr = getIpFromDomain(domain);
	}
}


/*
* 通过目标域名获取目的服务器ipv4地址并返回
* return: 目的服务器ipv4地址
*/
char* Task::getIpFromDomain(string domain)
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
		return getIpv6(domain, hints, ipBuf);		/* 该域名未解析出ipv4地址，则尝试获得域名的ipv6地址 */
	}

	PSOCKADDR_IN addr;
	ipBuf = new char[ADDRLEN_IPV4];
	addr = (PSOCKADDR_IN)res->ai_addr;				/* 拥有多个ip地址时只保存第一个 */
	inet_ntop(AF_INET, &addr->sin_addr, ipBuf, ADDRLEN_IPV4);
	return ipBuf;
}


/*
* 尝试解析出域名对应的ipv6地址，存入ipBuf中并返回
*/
char* Task::getIpv6(string domain, ADDRINFOA& hints, char* ipBuf)
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
	printf("\033[1;34m It's really ipv6!!!\033[0m\n");		//hack Debug info
	return ipBuf;
}
