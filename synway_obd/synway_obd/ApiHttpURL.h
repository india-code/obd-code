#pragma once

#include <afxmt.h>
#include "curl\curl.h"
#include <string>
#include<vector>

class ApiHttpURL
{
private:
	CURL * curlPtr;
public:
	static CMutex mMutex;
	static std::vector<std::string> vecHttpURL;
	static std::string readBuffer;

	ApiHttpURL();

	void PushBackURL(std::string url);
	std::string PopFrontURL();
	CURLcode callApi(const char *);
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
	~ApiHttpURL();
};

