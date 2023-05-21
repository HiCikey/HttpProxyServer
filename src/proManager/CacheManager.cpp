#include "managers.h"
using namespace managers;

CacheManager::CacheManager()
{

}

CacheManager::~CacheManager()
{
	for (CacheLabelPtr ptr : cache) {
		delete ptr->data;
		delete ptr;
	}
}

void managers::CacheManager::addLable(std::string url, char* package, int packLen)
{
	CacheLabelPtr newLable = new CacheLabel();
	newLable->data = new char[packLen];
	memcpy(newLable->data, package, packLen);
	newLable->data[packLen] = 0;
	newLable->url = url;
	newLable->size = packLen;
	newLable->last = 0;
	newLable->weight = 0;
	std::unique_lock<std::mutex> lck(mtx_cache);
	cache.emplace(newLable);
}

std::pair<char*, int> managers::CacheManager::serchLable(std::string url)
{
	std::unique_lock<std::mutex>lck(mtx_cache);
	for (CacheLabelPtr ptr : cache) {
		lck.unlock();
		if (ptr->url != url) {
			lck.lock();
			continue;
		}
		return std::pair<char*, int>(ptr->data, ptr->size);
	}
	return std::pair<char*, int>(nullptr, -1);
}

