﻿#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include <time.h>

#define BUFFER_SIZE 1024*1024	/* 报文缓冲区大小 */
#define ADDRLEN_IPV4 16			/* ipv4地址缓冲区大小 */
#define ADDRLEN_IPV6 46			/* ipv6地址缓冲区大小 */

static std::mutex mtx_tasks;	/* 访问任务列表的互斥锁 */

// 客户端信息结构体
typedef struct ClientInfo {
	char* addr = NULL;			/* IP地址 */
	unsigned short port = 0;	/* 端口号 */
}*ClientInfoPtr;


// 服务器端信息结构体
typedef struct ServerInfo {
	char* addr = NULL;			/* IP地址 */
	unsigned short port = 0;	/* 端口号 */
	std::string domain;			/* 域名 */
}*ServerInfoPtr;


// 缓存标签，可通过该结构体唯一标识一个缓存文件
typedef struct CacheLabel {
	char* data;				/* 缓存文件内容指针 */
	time_t last;			/* 该缓存上次被访问的时间戳 */
	unsigned int size;		/* 文件大小（字节） */
	std::string url;
	double weight;
}*CacheLabelPtr;


enum model {
	MODEL_IP,
	MODEL_DOMAIN,
	MODEL_TYPE
};

// 打印当前时间
static void printTime()
{
	std::string str = "";
	time_t timer;
	struct tm t = { 0 };
	time(&timer);
	localtime_s(&t, &timer);
	if (t.tm_hour < 10)
		str = "0";
	str = str + std::to_string(t.tm_hour) + ":";
	if (t.tm_min < 10)
		str += "0";
	str = str + std::to_string(t.tm_min) + ":";
	if (t.tm_sec < 10)
		str += "0";
	str = str + std::to_string(t.tm_sec);
	printf("%s  ", str.c_str());
}
