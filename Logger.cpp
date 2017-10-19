/**
 * @file	Logger.cpp
 * @class	Logger
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/11/02
 * @brief	Logger
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "Logger.h"

#ifdef JSCUTILS_OS_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

#ifdef HAS_SYSLOG
#include <syslog.h>
#endif

namespace JsCPPUtils
{
	Logger::Logger()
	{
		m_pParent = NULL;
		m_outtype = TYPE_NULL;
		m_fp = NULL;
		m_cbfunc = NULL;
		m_cbuserptr = NULL;
		m_lasterrno = 0;
	}

	Logger::Logger(OutputType outputType, const char *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr)
	{
		init(outputType, szFilePath, cbfunc, cbuserptr);
	}

	void Logger::close()
	{
		if (m_fp != NULL)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
		m_pParent = NULL;
		m_outtype = TYPE_NULL;
		m_fp = NULL;
		m_cbfunc = NULL;
		m_cbuserptr = NULL;
		m_lasterrno = 0;
	}

	int Logger::init(OutputType outputType, const char *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr)
	{
		int retval = 1;
		close();
		m_lasterrno = 0;

		switch(outputType)
		{
		case TYPE_STDOUT:
			m_fp = stdout;
			break;
		case TYPE_STDERR:
			m_fp = stderr;
			break;
		case TYPE_FILE: {
#ifdef JSCUTILS_OS_WINDOWS
			HANDLE hFile = ::CreateFileA(szFilePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			int fd;
			
			if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE))
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			fd = ::_open_osfhandle((intptr_t)hFile, _O_RDWR); // _O_APPEND
			if (fd < 0)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			m_fp = _fdopen(fd, "a");
			if (m_fp == NULL)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				_close(fd);
				break;
			}
#else
			m_fp = fopen(szFilePath, "a+");
			if(m_fp == NULL)
				m_lasterrno = errno;
#endif
			}
			break;
		case TYPE_CALLBACK:
			m_cbfunc = cbfunc;
			m_cbuserptr = cbuserptr;
			break;
		}

		if(retval == 1)
			m_outtype = outputType;

		return retval;
	}

#if defined(JSCUTILS_OS_WINDOWS)
	Logger::Logger(OutputType outputType, const wchar_t *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr)
	{
		init(outputType, szFilePath, cbfunc, cbuserptr);
	}

	int Logger::init(OutputType outputType, const wchar_t *szFilePath, CallbackFunc_t cbfunc, void *cbuserptr)
	{
		int retval = 1;
		close();
		m_lasterrno = 0;

		switch (outputType)
		{
		case TYPE_STDOUT:
			m_fp = stdout;
			break;
		case TYPE_STDERR:
			m_fp = stderr;
			break;
		case TYPE_FILE: {
#ifdef JSCUTILS_OS_WINDOWS
			HANDLE hFile = ::CreateFileW(szFilePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			int fd;

			if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE))
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			fd = ::_open_osfhandle((intptr_t)hFile, _O_RDWR); // _O_APPEND
			if (fd < 0)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			m_fp = _fdopen(fd, "a");
			if (m_fp == NULL)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				_close(fd);
				break;
			}
#else
			m_fp = wfopen(szFilePath, L"a+");
			if (m_fp == NULL)
				m_lasterrno = errno;
#endif
		}
		break;
		case TYPE_CALLBACK:
			m_cbfunc = cbfunc;
			m_cbuserptr = cbuserptr;
			break;
		}

		if (retval == 1)
			m_outtype = outputType;

		return retval;
	}
#endif

	Logger::Logger(Logger *pParent, const std::string& strPrefixName)
	{
		init(pParent, strPrefixName);
	}
	Logger::Logger(const JsCPPUtils::SmartPointer<Logger> &spParent, const std::string& strPrefixName)
	{
		init(spParent, strPrefixName);
	}

	int Logger::init(Logger *pParent, const std::string& strPrefixName)
	{
		close();
		m_spParent = NULL;
		m_pParent = pParent;
		m_strPrefixName = strPrefixName;
		return 1;
	}
	int Logger::init(const JsCPPUtils::SmartPointer<Logger> &spParent, const std::string& strPrefixName)
	{
		close();
		m_spParent = spParent;
		m_pParent = spParent.getPtr();
		m_strPrefixName = strPrefixName;
		return 1;
	}


	Logger::~Logger()
	{
		close();
	}
	
	void Logger::setParent(Logger *pParent)
	{
		m_spParent = NULL;
		m_pParent = pParent;
	}
	void Logger::setParent(const JsCPPUtils::SmartPointer<Logger> &spParent)
	{
		m_spParent = spParent;
		m_pParent = spParent.getPtr();
	}
	
	void Logger::_child_puts(LogType logtype, const std::string& strPrefixName, const char* szLog)
	{
		if(m_pParent != NULL)
		{
			m_pParent->_child_puts(logtype, m_strPrefixName + ": " + strPrefixName, szLog);
		}else{
			printf(logtype, "%s: %s", strPrefixName.c_str(), szLog);
		}
	}

	void Logger::printf(LogType logtype, const char* format, ...)
	{
		static char strmonths[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		
		long buf1size = 1536, buf2size = 1536;
#ifdef JSLOGGER_USE_STACK
		char pbuf1[1536];
		char pbuf2[1536];
#else
		char *pbuf1 = NULL;
		char *pbuf2 = NULL;
#endif

		va_list args;
	
		time_t rawtime;
		struct tm timeinfo;

		const char *strlogtype = NULL;
		const char *stroutput = NULL;

		do {
#ifndef JSLOGGER_USE_STACK
			pbuf1 = (char*)malloc(buf1size);
			if(pbuf1 == NULL)
				break;
#endif
			
			time(&rawtime);
#if defined(JSCUTILS_OS_WINDOWS)
			localtime_s(&timeinfo, &rawtime);
#elif defined(JSCUTILS_OS_LINUX)
			localtime_r(&rawtime, &timeinfo);
#endif

			va_start(args, format);
#ifdef _JSCUTILS_MSVC_CRT_SECURE
			vsprintf_s(pbuf1, buf1size, format, args);
#else
			vsprintf(pbuf1, format, args);
#endif
			va_end(args);

			
			if(m_pParent != NULL)
			{
				m_pParent->_child_puts(logtype, m_strPrefixName, pbuf1);
			}else{
				switch(logtype)
				{
				case LOGTYPE_EMERG:
					strlogtype = "EMERG";
					break;
				case LOGTYPE_ALERT:
					strlogtype = "ALERT";
					break;
				case LOGTYPE_CRIT:
					strlogtype = "CRIT";
					break;
				case LOGTYPE_ERR:
					strlogtype = "ERR";
					break;
				case LOGTYPE_WARNING:
					strlogtype = "WARN";
					break;
				case LOGTYPE_NOTICE:
					strlogtype = "NOTI";
					break;
				case LOGTYPE_INFO:
					strlogtype = "INFO";
					break;
				case LOGTYPE_DEBUG:
					strlogtype = "DEBUG";
					break;
				default:
					strlogtype = "UNDEFINED";
					break;
				}

				switch(m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
				case TYPE_CALLBACK:
#ifndef JSLOGGER_USE_STACK
					pbuf2 = (char*)malloc(buf1size);
					if(pbuf2 == NULL)
						break;
#endif
#ifdef JSCUTILS_OS_WINDOWS
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					sprintf_s(pbuf2, buf2size, "[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
#else
					sprintf(pbuf2, "[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
#endif
#else
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					sprintf_s(pbuf2, buf2size, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
#else
					sprintf(pbuf2, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, pbuf1);
#endif
#endif
					stroutput = pbuf2;
					break;
				case TYPE_SYSLOG:
					stroutput = pbuf1;
					break;
				default:
					stroutput = NULL;
					break;
				}

				if(stroutput == NULL)
					break;

				lock();
				switch(m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
					fputs(stroutput, m_fp);
					fflush(m_fp);
					break;
				case TYPE_CALLBACK:
					m_cbfunc(m_cbuserptr, stroutput);
					break;
				case TYPE_SYSLOG:
	#ifdef HAS_SYSLOG
					syslog(logtype, "%s", stroutput);
	#endif
					break;
				default:
					break;
				}
				unlock();
			}

		}while(0);
		
#ifndef JSLOGGER_USE_STACK
		if(pbuf1 != NULL)
		{
			free(pbuf1);
			pbuf1 = NULL;
		}
		if(pbuf2 != NULL)
		{
			free(pbuf2);
			pbuf2 = NULL;
		}
#endif
	}
}
