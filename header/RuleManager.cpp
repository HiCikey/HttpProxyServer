#include"managers.h"

using namespace managers;

RuleManager::RuleManager()
{
	// 初始化并连接mysql服务器
	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");
}

RuleManager::~RuleManager()
{
	mysql_close(&mysql);
}


std::set<std::string> RuleManager::getIpList()
{
	std::unique_lock<std::mutex> lck(mtx_ip);
	return black_ip;
}

std::set<std::string> RuleManager::getDomainList()
{
	std::unique_lock<std::mutex> lck(mtx_domain);
	return black_domain;
}

std::set<std::string> RuleManager::getTypeList()
{
	std::unique_lock<std::mutex> lck(mtx_type);
	return black_type;
}


/*
* 客户端IP黑名单中增加一项
*/
bool RuleManager::addIp(std::string str)
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
* 域名黑名单中增加一项
*/
bool RuleManager::addDomain(std::string str)
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
* 传输文件类型黑名单中增加一项
*/
bool RuleManager::addType(std::string str)
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
* 客户端IP黑名单中删除一项
*/
void RuleManager::deleteIp(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_ip);
	if (black_ip.count(str) == 0)
		return;
	black_ip.erase(str);
	lck.unlock();

	std::string query = "delete from black_ip where ip='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


/*
* 域名黑名单中删除一项
*/
void RuleManager::deleteDomain(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_domain);
	if (black_domain.count(str) == 0)
		return;
	black_domain.erase(str);
	lck.unlock();

	std::string query = "delete from black_domain where domain='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


/*
* 传输文件类型黑名单中删除一项
*/
void RuleManager::deleteType(std::string str)
{
	std::unique_lock<std::mutex> lck(mtx_type);
	if (black_type.count(str) == 0)
		return;
	black_type.erase(str);
	lck.unlock();

	std::string query = "delete from black_type where type='" + str + "';";
	mysql_query(&mysql, query.c_str());
}


/*
* 访问数据库获取初始黑名单
*/
bool RuleManager::setBlackList()
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

void RuleManager::setDomainList()
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

void RuleManager::setIpList()
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

void RuleManager::setTypeList()
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
