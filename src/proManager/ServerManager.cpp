#include "managers.h"
using namespace managers;

ServerManager::ServerManager()
{
	sockServer = INVALID_SOCKET;
	serverHost = new ServerInfo();
}

ServerManager::~ServerManager()
{
	if (sockServer != INVALID_SOCKET)
		closesocket(sockServer);
	delete serverHost->addr;
}


/*
* 请求与目的web服务器建立tcp连接，基于ipv4协议
*/
bool ServerManager::connectServer(SOCKET& sock)
{
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockServer == INVALID_SOCKET) {
		printf("\033[1;31m[ERROR] Failed to create server socket\033[0m\n");
		return false;
	}
	sock = sockServer;

	int result;
	SOCKADDR_IN sockAddrServer{};		/* socket通信地址 */
	sockAddrServer.sin_family = AF_INET;
	sockAddrServer.sin_port = htons(serverHost->port);
	sockAddrServer.sin_addr.S_un.S_addr = inet_addr(serverHost->addr);

	/* 发起tcp连接请求 */
	result = connect(sockServer, (SOCKADDR*)&sockAddrServer, sizeof(sockAddrServer));
	if (result == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Failed to connect with server [%s]%s:%u, code = %d\033[0m\n", serverHost->addr, serverHost->domain.c_str(), serverHost->port, WSAGetLastError());
		return false;
	}
	return true;
}

/*
* 请求与目的web服务器建立tcp连接，基于ipv6协议
*/
bool managers::ServerManager::connectIpv6Server(SOCKET& sock)
{
	sockServer = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (sockServer == INVALID_SOCKET) {
		printf("\033[1;31m[ERROR] Failed to create server socket\033[0m\n");
		return false;
	}
	sock = sockServer;

	int result;
	SOCKADDR_IN6 sockAddrServer{};		/* socket通信地址 */
	sockAddrServer.sin6_family = AF_INET6;
	sockAddrServer.sin6_port = htons(serverHost->port);
	inet_pton(AF_INET6, serverHost->addr, &sockAddrServer.sin6_addr);

	/* 发起tcp连接请求 */
	result = connect(sockServer, (SOCKADDR*)&sockAddrServer, sizeof(sockAddrServer));
	if (result == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Failed to connect with server [%s]%s:%u, code = %d\033[0m\n", serverHost->addr, serverHost->domain.c_str(), serverHost->port, WSAGetLastError());
		return false;
	}
	return true;
}


/*
* 接收服务器的回复报文
* buf: 报文缓冲区
* packLen: 报文长度
* return: 报文内容
*/
bool ServerManager::recvFromServer(char* buf, int& packLen)
{
	printTime();
	packLen = recv(sockServer, buf, BUFFER_SIZE - 1, 0);
	if (packLen > 0 && packLen < BUFFER_SIZE) {
		buf[packLen] = 0;
		printf("Receive %d bytes from server [%s]%s:%u\n", packLen, serverHost->addr, serverHost->domain.c_str(), serverHost->port);
		return true;
	}
	else if (packLen == 0) {
		printf("\033[2;33m[WARNING] The connection is over from server [%s]%s:%u\033[0m\n", serverHost->addr, serverHost->domain.c_str(), serverHost->port);
		return false;
	}
	else {
		printf("\033[1;31m[ERROR] Recv failed from server [%s]%s:%u, code = %d\033[0m\n", serverHost->addr, serverHost->domain.c_str(), serverHost->port, WSAGetLastError());
		return false;
	}
}


/*
* 转发客户端的请求报文给服务器
* buf: 报文缓冲区
* size: 报文大小
*/
bool ServerManager::sendToServer(char* buf, int size)
{
	printTime();
	int res = send(sockServer, buf, size, 0);
	if (res == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Fail to send to server [%s]%s:%u, code = %d\033[0m\n", serverHost->addr, serverHost->domain.c_str(), serverHost->port, WSAGetLastError());
		return false;
	}
	printf("Send %d bytes to server [%s]%s:%u\n", size, serverHost->addr, serverHost->domain.c_str(), serverHost->port);
	return true;
}
