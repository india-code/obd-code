#include "stdafx.h"
#include "ApiHttpURL.h"

std::string ApiHttpURL::readBuffer("");

ApiHttpURL::ApiHttpURL()
{
	curlPtr = curl_easy_init();	
}

CURLcode ApiHttpURL::callApi(const char* uRL)
{
	CURLcode res;
	readBuffer.clear();
	curl_easy_setopt(curlPtr, CURLOPT_URL, uRL);
	curl_easy_setopt(curlPtr, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curlPtr, CURLOPT_WRITEFUNCTION, WriteCallback);
	res = curl_easy_perform(curlPtr);
	return res;
}

size_t ApiHttpURL::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	readBuffer.append((char*)contents, realsize);
	return realsize;
}

ApiHttpURL::~ApiHttpURL()
{
	curl_easy_cleanup(curlPtr);
}