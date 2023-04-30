#include "ProxyServer.h"

ProxyServer::ProxyServer()
{
	sockListen = INVALID_SOCKET;
	sockAddrListen = { 0 };
	pool = nullptr;

	// 初始化并连接mysql服务器
	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");
}

ProxyServer::~ProxyServer()
{
	delete pool;
}


void ProxyServer::proxyStartUp()
{
	if (!initial() || !setBlackList())
		return;
	pool = new ThreadPool<Task>(THREAD_COUNT);
	std::unique_lock<std::mutex> lck(mtx_tasks);
	lck.unlock();
	while (true)
	{
		// 从监听队列中取出一个连接请求，若无则阻塞
		SOCKADDR_IN addr{};
		int len = sizeof(SOCKADDR_IN);
		SOCKET sockConn = accept(sockListen, (SOCKADDR*)&addr, &len);
		if (sockConn == INVALID_SOCKET)
			continue;

		// 新建一个任务并交给线程池处理
		Task::count++;
		Task* task = new Task(sockConn, addr);
		if (task != NULL) {
			pool->addTask(task);
			lck.lock();
			tasks.emplace_back(task);
			lck.unlock();
		}
	}
	closesocket(sockListen);
	WSACleanup();
	mysql_close(&mysql);
}

void ProxyServer::deleteTask(int taskSeq)
{
	Task* task = nullptr;
	std::unique_lock<std::mutex> lck(mtx_tasks);
	task = tasks.at(taskSeq);
	tasks.erase(tasks.begin() + taskSeq);
	lck.unlock();
	delete task;
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
		std::cout << "WSAStartup failed!!!" << std::endl;
		return false;
	}
	std::cout << "Succeed to initiate Winsock library...\n\n";
	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
		std::cout << "Cannot find specified Winsock library version!!!" << std::endl;
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

/*
* 域名黑名单中增加一项
*/
bool ProxyServer::addDomain(std::string str)
{
	// 插入内存
	std::unique_lock<std::mutex> lck(mtx_domain);
	if (black_domain.count(str) == 1)
		return false;
	black_domain.emplace(str);
	lck.unlock();

	// 插入本地数据库
	std::string query = "insert into black_domain values('" + str + "');";
	mysql_query(&mysql, query.c_str());
	return true;
}

/*
* 域名黑名单中删除一项
*/
void ProxyServer::deleteDomain(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_domain);
	if (black_domain.count(str) == 0)
		return;
	black_domain.erase(str);
	lck.unlock();

	std::string query = "delete from black_domain where domain='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


std::set<std::string> ProxyServer::getDomainList()
{
	std::unique_lock<std::mutex> lck(mtx_domain);
	return black_domain;
}

/*
* 客户端IP黑名单中增加一项
*/
bool ProxyServer::addIp(std::string str)
{
	// 插入内存
	std::unique_lock<std::mutex> lck(mtx_ip);
	if (black_ip.count(str) == 1)
		return false;
	black_ip.emplace(str);
	lck.unlock();
	
	// 插入本地数据库
	std::string query = "insert into black_ip values('" + str + "');";
	mysql_query(&mysql, query.c_str());
	return true;
}

/*
* 客户端IP黑名单中删除一项
*/
void ProxyServer::deleteIp(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_ip);
	if (black_ip.count(str) == 0)
		return;
	black_ip.erase(str);
	lck.unlock();

	std::string query = "delete from black_ip where ip='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


std::set<std::string> ProxyServer::getIpList()
{
	std::unique_lock<std::mutex> lck(mtx_ip);
	return black_ip;
}

/*
* 传输文件类型黑名单中增加一项
*/
bool ProxyServer::addType(std::string str)
{
	// 插入内存
	std::unique_lock<std::mutex> lck(mtx_type);
	if (black_type.count(str) == 1)
		return false;
	black_type.emplace(str);
	lck.unlock();

	// 插入本地数据库
	std::string query = "insert into black_type values('" + str + "');";
	mysql_query(&mysql, query.c_str());
	return true;
}

/*
* 传输文件类型黑名单中删除一项
*/
void ProxyServer::deleteType(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_type);
	if (black_type.count(str) == 0)
		return;
	black_type.erase(str);
	lck.unlock();

	std::string query = "delete from black_type where type='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


std::set<std::string> ProxyServer::getTypeList()
{
	std::unique_lock<std::mutex> lck(mtx_type);
	return black_type;
}

/*
* 访问数据库获取初始黑名单
*/
bool ProxyServer::setBlackList()
{
	if (mysql_real_connect(&mysql, "localhost", "root", "SQL_rt596@Royel", "proxyrule", 3306, NULL, 0) == NULL) {
		printf("\033[1;31m[ERROR] at connect mysql...\033[0m\n");
		return false;
	}
	setDomainList();
	setIpList();
	setTypeList();
	return true;
}

void ProxyServer::setDomainList()
{
	MYSQL_RES* res = nullptr;
	MYSQL_ROW row = nullptr;
	std::unique_lock<std::mutex> lck(mtx_domain);
	lck.unlock();

	// 查询数据库中的域名黑名单
	mysql_query(&mysql, "select * from black_domain;");
	res = mysql_store_result(&mysql);
	while (row = mysql_fetch_row(res)) {
		lck.lock();
		black_domain.emplace(std::string(row[0]));
		lck.unlock();
	}
	mysql_free_result(res);
}

void ProxyServer::setIpList()
{
	MYSQL_RES* res = nullptr;
	MYSQL_ROW row = nullptr;
	std::unique_lock<std::mutex> lck(mtx_ip);
	lck.unlock();

	// 查询数据库中的IP黑名单
	mysql_query(&mysql, "select * from black_ip;");
	res = mysql_store_result(&mysql);
	while (row = mysql_fetch_row(res)) {
		lck.lock();
		black_ip.emplace(std::string(row[0]));
		lck.unlock();
	}
	mysql_free_result(res);
}

void ProxyServer::setTypeList()
{
	MYSQL_RES* res = nullptr;
	MYSQL_ROW row = nullptr;
	std::unique_lock<std::mutex> lck(mtx_type);
	lck.unlock();

	// 查询数据库中的传输文件类型黑名单
	mysql_query(&mysql, "select * from black_type;");
	res = mysql_store_result(&mysql);
	while (row = mysql_fetch_row(res)) {
		lck.lock();
		black_type.emplace(std::string(row[0]));
		lck.unlock();
	}
	mysql_free_result(res);
}
