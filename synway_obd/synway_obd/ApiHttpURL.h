#pragma once

#include "curl\curl.h"
#include <string>

class ApiHttpURL
{
private:
	CURL * curlPtr;
public:
	static std::string readBuffer;
	ApiHttpURL();
	CURLcode callApi(const char *);
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
	~ApiHttpURL();
};

