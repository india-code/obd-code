#pragma once
#include "stdafx.h"

class Utils
{
public:
	static char* GetIPAdd()
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		char Name[255];
		PHOSTENT HostInfo;
		wVersionRequested = MAKEWORD(1, 1);
		char *IPAdd;
		if (WSAStartup(wVersionRequested, &wsaData) == 0)
		{
			if (gethostname(Name, sizeof(Name)) == 0)
			{
				//printf("Host name: %s\n", name);
				if ((HostInfo = gethostbyname(Name)) != NULL)
				{
					int nCount = 0;
					while (HostInfo->h_addr_list[nCount])
					{
						IPAdd = inet_ntoa(*(struct in_addr *)HostInfo->h_addr_list[nCount]);

						++nCount;
					}
				}
			}
		}
		return IPAdd;
	}
};