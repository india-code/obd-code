// logger.h : is being used for generate logs
#pragma once
#include <sys/timeb.h>
#include "zip.h"

enum LogLevel
{
	LOGTRACE = 0,
	LOGDEBUG,
	LOGINFO,
	LOGWARNING,
	LOGERR,
	LOGFATAL
};

class CLogger {
	int MinLogLevel;
	FILE *fp;
	char CurFilePath[256];
public:
	tm getTime(char time_value[25]) {
		struct tm                  Time;
		time_t                     Timet;

#ifdef _WIN32 /* Defined in Windows */  
		struct _timeb   Timeb; /* used to get milliseconds */
#else                /* Defined in Unix   */
		struct timeb         Timeb;
#endif

		char TimeStr[50];
		char buffer[50];

		buffer[0] = '\0';
		TimeStr[0] = '\0';
		time(&Timet);
		localtime_s(&Time, &Timet);

#ifdef _WIN32 /* Defined in Windows */  
		_ftime_s(&Timeb); /* needed to get the milliseconds */
#else                /* Defined in Unix   */  
		ftime(&Timeb);
#endif
		sprintf_s(TimeStr, "%02d/%02d/%04d %02d:%02d:%02d.%03d ", Time.tm_mday, Time.tm_mon + 1, Time.tm_year + 1900, Time.tm_hour, Time.tm_min, Time.tm_sec, Timeb.millitm);
		StrCpyA(time_value, TimeStr);
		return Time;
	}
	CLogger()
	{
		openLoggerFile();
	}
	void zipCurrentFile(char* fileName)
	{
		HZIP hz;
		char CurPath[256];
		char folderName[50];
		char timeValue[25];

		GetCurrentDirectoryA(200, CurPath);
		StrCatA(CurPath, "\\ApplicationLogs");
		if (!PathIsDirectoryA(CurPath))
		{
			CreateDirectoryA(CurPath, NULL);
		}
		tm timeVal = getTime(timeValue);
		sprintf_s(folderName, "\\Application_%04d%02d%02d", timeVal.tm_year + 1900, timeVal.tm_mon + 1, timeVal.tm_mday);
		StrCatA(CurPath, folderName);

		if (!PathIsDirectoryA(CurPath))
		{
			CreateDirectoryA(CurPath, NULL);
		}
		StrCatA(CurPath, "\\");
		StrCatA(CurPath, fileName);
		StrCatA(CurPath, ".zip");
		CString fileNameW(fileName);
		CString zipName(CurPath);
		hz = CreateZip(zipName, 0);

		ZipAdd(hz, fileNameW, fileNameW);
		
		CloseZip(hz);
		remove(fileName);
	}
	void openLoggerFile()
	{
		char fileName[50], timeValue[25];

		tm timeVal = getTime(timeValue);
		sprintf_s(fileName, "Application_%02d%02d%02d.log", timeVal.tm_hour, timeVal.tm_min, timeVal.tm_sec);
		StrCpyA(CurFilePath, fileName);
		fopen_s(&fp, fileName, "a");
	}
	void SetMinLogLevel(int minLogLevel)
	{
		MinLogLevel = minLogLevel;
	}
	void log(LogLevel logger, const char* format...) {

		char* time_value;
		int log_level = 0;
		va_list args;

		time_value = (char*)malloc(26 * sizeof(char));
		getTime(time_value);
		va_start(args, format);
		switch (logger) {
		case 0:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Trace   : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		case 1:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Debug   : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		case 2:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Info    : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		case 3:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Warning : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		case 4:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Error   : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		case 5:if ((int)logger >= MinLogLevel) { fprintf(fp, time_value); fprintf(fp, " Fatal   : "); vfprintf(fp, format, args); fprintf(fp, "\n"); break; }
		}
		va_end(args);
		if (ftell(fp) >= 10000000)
		{
			fclose(fp);
			zipCurrentFile(CurFilePath);
			openLoggerFile();
		}
		fflush(fp);
	}

	~CLogger()
	{
		log(LOGINFO, "Destructor called!!! %s", CurFilePath);
		fclose(fp);
		zipCurrentFile(CurFilePath);
	}
};
