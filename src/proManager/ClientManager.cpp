#include "managers.h"
using namespace managers;

ClientManager::ClientManager(SOCKET sockConn, SOCKADDR_IN addr)
{
	sockClient = sockConn;
	clientHost = new ClientInfo();
	clientHost->addr = new char[ADDRLEN_IPV4];
	inet_ntop(AF_INET, &addr.sin_addr, clientHost->addr, ADDRLEN_IPV4);
	clientHost->port = ntohs(addr.sin_port);
}

managers::ClientManager::ClientManager(SOCKET sockConnIpv6, SOCKADDR_IN6 addrIpv6)
{
	sockClient = sockConnIpv6;
	clientHost = new ClientInfo();
	clientHost->addr = new char[ADDRLEN_IPV6];
	inet_ntop(AF_INET6, &addrIpv6.sin6_addr, clientHost->addr, ADDRLEN_IPV6);
	clientHost->port = ntohs(addrIpv6.sin6_port);
}

ClientManager::~ClientManager()
{
	closesocket(sockClient);
	delete[] clientHost->addr;
	delete clientHost;
}

/*
* 接收客户端的请求报文
* buf: 报文缓冲区
* packLen: 报文长度
* return: 报文内容
*/
void ClientManager::recvFromClient(char* buf, int& packLen)
{
	printTime();
	packLen = recv(sockClient, buf, BUFFER_SIZE, 0);
	if (packLen > 0)
		printf("Receive %d bytes from client %s:%u\n", packLen, clientHost->addr, clientHost->port);
	else if (packLen == 0)
		printf("\033[2;33m[WARNING] The connection is over from client %s:%u\033[0m\n", clientHost->addr, clientHost->port);
	else
		printf("\033[1;31m[ERROR] Recv failed from client %s:%u, code = %d\033[0m\n", clientHost->addr, clientHost->port, WSAGetLastError());
}


/*
* 回复客户端
* buf: 报文缓冲区
* size: 报文大小
*/
bool ClientManager::sendToClient(char* buf, int size)
{
	printTime();
	int res = send(sockClient, buf, size, 0);
	if (res == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Fail to send to client %s:%u, code = %d\033[0m\n", clientHost->addr, clientHost->port, WSAGetLastError());
		return false;
	}
	printf("Send %d bytes to client %s:%u\n", size, clientHost->addr, clientHost->port);
	return true;
}
