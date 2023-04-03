#include "ProxyServer.h"

ProxyServer::ProxyServer()
{
	sockListen = INVALID_SOCKET;
	sockAddrListen = { 0 };
	pool = nullptr;
}

ProxyServer::~ProxyServer()
{
	delete pool;
}


void ProxyServer::proxyStartUp()
{
	if (!initial()) return;
	pool = new ThreadPool<Task>(THREAD_COUNT);
	while (true)
	{
		// 从监听队列中取出一个连接请求，若无则阻塞
		SOCKADDR_IN addr{};
		int len = sizeof(SOCKADDR_IN);
		SOCKET sockConn = accept(sockListen, (SOCKADDR*)&addr, &len);
		if (sockConn == INVALID_SOCKET)
			continue;

		Task::count++;
		Task* task = new Task(sockConn, addr);
		pool->addTask(task);
	}
	closesocket(sockListen);
	WSACleanup();
}


/*
* 返回当前任务队列，用于主窗口显示当前各客户端-服务器连接状态
*/
queue<Task*> ProxyServer::getTasks()
{
	if (pool == nullptr)
		return queue<Task*>();
	else
		return pool->getTaskQueue();
}


/*
* 加载Winsock库
* 调用函数创建监听socket
*/
bool ProxyServer::initial()
{
	// 加载Winsock库
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		cout << "WSAStartup failed!!!" << endl;
		return false;
	}
	cout << "Succeed to initiate Winsock library...\n\n";
	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
		cout << "Cannot find specified Winsock library version!!!" << endl;
		WSACleanup();
		return false;
	}
	if (!creatSocket()) {
		printf("\033[1;31m[ERROR] Failed to start up Proxy Server......\033[0m\n");
		WSACleanup();
		return false;
	}
	return true;
}


/*
* 创建监听ipv4的socket，绑定通信地址，代理服务器开始监听
*/
bool ProxyServer::creatSocket()
{
	// 创建监听ipv4客户端的socket
	sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockListen == INVALID_SOCKET) {
		printf("\033[1;31m[ERROR] At create ipv4 listen socket(), code= %d\033[0m\n", WSAGetLastError());
		return false;
	}

	// 绑定套接字与通信地址
	sockAddrListen.sin_family = AF_INET;
	sockAddrListen.sin_port = htons(LISTEN_PORT);
	sockAddrListen.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(sockListen, (SOCKADDR*)&sockAddrListen, sizeof(sockAddrListen)) == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Failed to bind ipv4 socket\033[0m\n");
		closesocket(sockListen);
		return false;
	}

	// 代理服务器开始监听客户端的http请求
	if (listen(sockListen, LISTEN_COUNT) == SOCKET_ERROR) {
		printf("\033[1;31m[ERROR] Failed to listen ipv4 request\033[0m\n");
		closesocket(sockListen);
		return false;
	}
	return true;
}
