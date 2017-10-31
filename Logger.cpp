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
#include "StringEncoding.h"
#endif

#ifdef HAS_SYSLOG
#include <syslog.h>
#endif

namespace JsCPPUtils
{
	Logger::Logger()
	{
		_clearInstance();
	}

	void Logger::_clearInstance()
	{
		m_pParent = NULL;
		m_outtype = TYPE_NULL;
		m_fp = NULL;
		m_cbfuncA = NULL;
		m_cbfuncW = NULL;
		m_cbuserptr = NULL;
		m_lasterrno = 0;
	}

	Logger::Logger(OutputType outputType, const char *szFilePath, CallbackFuncA_t cbfuncA, CallbackFuncW_t cbfuncW, void *cbuserptr)
	{
		_clearInstance();
		init(outputType, szFilePath, cbfuncA, cbfuncW, cbuserptr);
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
		m_cbfuncA = NULL;
		m_cbfuncW = NULL;
		m_cbuserptr = NULL;
		m_lasterrno = 0;
	}

	int Logger::init(OutputType outputType, const char *szFilePath, CallbackFuncA_t cbfuncA, CallbackFuncW_t cbfuncW, void *cbuserptr)
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
			unsigned char bom[2] = { 0xFF, 0xFE };
			DWORD dwWinErr = 0;
			HANDLE hFile;
			int fd;
			::SetLastError(0);
			hFile = ::CreateFileA(szFilePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE))
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			dwWinErr = ::GetLastError();
			if (dwWinErr != ERROR_ALREADY_EXISTS)
			{
				DWORD dwWrittenBytes = 0;
				::WriteFile(hFile, bom, sizeof(bom), &dwWrittenBytes, NULL);
			}

			fd = ::_open_osfhandle((intptr_t)hFile, _O_RDWR);
			if (fd < 0)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			m_fp = _fdopen(fd, "a,ccs=UTF-8");
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
			m_cbfuncA = cbfuncA;
			m_cbfuncW = cbfuncW;
			m_cbuserptr = cbuserptr;
			break;
		}

		if(retval == 1)
			m_outtype = outputType;

		return retval;
	}

#if defined(JSCUTILS_OS_WINDOWS)
	Logger::Logger(OutputType outputType, const wchar_t *szFilePath, CallbackFuncA_t cbfuncA, CallbackFuncW_t cbfuncW, void *cbuserptr)
	{
		_clearInstance();
		init(outputType, szFilePath, cbfuncA, cbfuncW, cbuserptr);
	}

	int Logger::init(OutputType outputType, const wchar_t *szFilePath, CallbackFuncA_t cbfuncA, CallbackFuncW_t cbfuncW, void *cbuserptr)
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
			unsigned char bom[2] = {0xFF, 0xFE};
			DWORD dwWinErr = 0;
			HANDLE hFile;
			int fd;
			::SetLastError(0);
			hFile = ::CreateFileW(szFilePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			
			if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE))
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			dwWinErr = ::GetLastError();
			if (dwWinErr != ERROR_ALREADY_EXISTS)
			{
				DWORD dwWrittenBytes = 0;
				::WriteFile(hFile, bom, sizeof(bom), &dwWrittenBytes, NULL);
			}

			fd = ::_open_osfhandle((intptr_t)hFile, _O_RDWR);
			if (fd < 0)
			{
				DWORD dwErr = ::GetLastError();
				m_lasterrno = dwErr;
				retval = -((int)dwErr);
				break;
			}
			m_fp = _fdopen(fd, "a,ccs=UNICODE");
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
			m_cbfuncA = cbfuncA;
			m_cbfuncW = cbfuncW;
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
		_clearInstance();
		init(pParent, strPrefixName);
	}
	Logger::Logger(const JsCPPUtils::SmartPointer<Logger> &spParent, const std::string& strPrefixName)
	{
		_clearInstance();
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
		if (m_pParent != NULL)
		{
			m_pParent->_child_puts(logtype, m_strPrefixName + ": " + strPrefixName, szLog);
		} else {
			this->printf(logtype, "%s: %s", strPrefixName.c_str(), szLog);
		}
	}

	void Logger::_child_puts(LogType logtype, const std::string& strPrefixName, const wchar_t* szLog)
	{
		if (m_pParent != NULL)
		{
			m_pParent->_child_puts(logtype, m_strPrefixName + ": " + strPrefixName, szLog);
		} else {
			this->printf(logtype, "%s: %s", strPrefixName.c_str(), szLog);
		}
	}

	void Logger::puts(LogType logtype, const char* text)
	{
		static char strmonths[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

		long buf2size = 1536;
#ifdef JSLOGGER_USE_STACK
		char pbuf2[1536];
#else
		char *pbuf2 = NULL;
#endif

		va_list args;

		time_t rawtime;
		struct tm timeinfo;

		const char *strlogtype = NULL;
		const char *stroutput = NULL;

		do {
			time(&rawtime);
#if defined(JSCUTILS_OS_WINDOWS)
			localtime_s(&timeinfo, &rawtime);
#elif defined(JSCUTILS_OS_LINUX)
			localtime_r(&rawtime, &timeinfo);
#endif

			if (m_pParent != NULL)
			{
				m_pParent->_child_puts(logtype, m_strPrefixName, text);
			}
			else {
				switch (logtype)
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

				switch (m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
				case TYPE_CALLBACK:
#ifndef JSLOGGER_USE_STACK
					pbuf2 = (char*)malloc(buf1size);
					if (pbuf2 == NULL)
						break;
#endif
#ifdef JSCUTILS_OS_WINDOWS
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					sprintf_s(pbuf2, buf2size, "[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#else
					sprintf(pbuf2, "[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#endif
#else
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					sprintf_s(pbuf2, buf2size, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#else
					sprintf(pbuf2, "[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#endif
#endif
					stroutput = pbuf2;
					break;
				case TYPE_SYSLOG:
					stroutput = text;
					break;
				default:
					stroutput = NULL;
					break;
				}

				if (stroutput == NULL)
					break;

				lock();
				switch (m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
#if defined(JSCUTILS_OS_WINDOWS)
				{
					fputws(StringEncoding::StringToUnicode(stroutput).c_str(), m_fp);
					fflush(m_fp);
				}
#else
					fputs(stroutput, m_fp);
					fflush(m_fp);
#endif
					break;
				case TYPE_CALLBACK:
					m_cbfuncA(m_cbuserptr, stroutput);
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

		} while (0);

#ifndef JSLOGGER_USE_STACK
		if (pbuf2 != NULL)
		{
			free(pbuf2);
			pbuf2 = NULL;
		}
#endif
	}

#if defined(JSCUTILS_OS_WINDOWS)
	void Logger::puts(LogType logtype, const wchar_t* text)
	{
		static wchar_t strmonths[][4] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

		long buf2size = 1536;
#ifdef JSLOGGER_USE_STACK
		wchar_t pbuf2[1536];
#else
		wchar_t *pbuf2 = NULL;
#endif

		va_list args;

		time_t rawtime;
		struct tm timeinfo;

		const wchar_t *strlogtype = NULL;
		const wchar_t *stroutput = NULL;

		do {
			time(&rawtime);
#if defined(JSCUTILS_OS_WINDOWS)
			localtime_s(&timeinfo, &rawtime);
#elif defined(JSCUTILS_OS_LINUX)
			localtime_r(&rawtime, &timeinfo);
#endif

			if (m_pParent != NULL)
			{
				m_pParent->_child_puts(logtype, m_strPrefixName, text);
			}
			else {
				switch (logtype)
				{
				case LOGTYPE_EMERG:
					strlogtype = L"EMERG";
					break;
				case LOGTYPE_ALERT:
					strlogtype = L"ALERT";
					break;
				case LOGTYPE_CRIT:
					strlogtype = L"CRIT";
					break;
				case LOGTYPE_ERR:
					strlogtype = L"ERR";
					break;
				case LOGTYPE_WARNING:
					strlogtype = L"WARN";
					break;
				case LOGTYPE_NOTICE:
					strlogtype = L"NOTI";
					break;
				case LOGTYPE_INFO:
					strlogtype = L"INFO";
					break;
				case LOGTYPE_DEBUG:
					strlogtype = L"DEBUG";
					break;
				default:
					strlogtype = L"UNDEFINED";
					break;
				}

				switch (m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
				case TYPE_CALLBACK:
#ifndef JSLOGGER_USE_STACK
					pbuf2 = (char*)malloc(buf1size);
					if (pbuf2 == NULL)
						break;
#endif
#ifdef JSCUTILS_OS_WINDOWS
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					swprintf_s(pbuf2, buf2size, L"[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#else
					swprintf(pbuf2, L"[%s] %s %02d %02d:%02d:%02d %d] %s\r\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#endif
#else
#ifdef _JSCUTILS_MSVC_CRT_SECURE
					swprintf_s(pbuf2, buf2size, L"[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#else
					swprintf(pbuf2, L"[%s] %s %02d %02d:%02d:%02d %d] %s\n", strlogtype, strmonths[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_year + 1900, text);
#endif
#endif
					stroutput = pbuf2;
					break;
				case TYPE_SYSLOG:
					stroutput = text;
					break;
				default:
					stroutput = NULL;
					break;
				}

				if (stroutput == NULL)
					break;

				lock();
				switch (m_outtype)
				{
				case TYPE_STDOUT:
				case TYPE_STDERR:
				case TYPE_FILE:
					fputws(stroutput, m_fp);
					fflush(m_fp);
					break;
				case TYPE_CALLBACK:
					m_cbfuncW(m_cbuserptr, stroutput);
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

		} while (0);

#ifndef JSLOGGER_USE_STACK
		if (pbuf2 != NULL)
		{
			free(pbuf2);
			pbuf2 = NULL;
		}
#endif
	}
#endif

	void Logger::printf(LogType logtype, const char* format, ...)
	{
		long buf1size = 1536;
#ifdef JSLOGGER_USE_STACK
		char pbuf1[1536];
#else
		char *pbuf1 = NULL;
#endif

		va_list args;

		do {
#ifndef JSLOGGER_USE_STACK
			pbuf1 = (char*)malloc(buf1size);
			if (pbuf1 == NULL)
				break;
#endif

			va_start(args, format);
#ifdef _JSCUTILS_MSVC_CRT_SECURE
			vsprintf_s(pbuf1, buf1size, format, args);
#else
			vsprintf(pbuf1, format, args);
#endif
			va_end(args);

			this->puts(logtype, pbuf1);
		} while (0);

#ifndef JSLOGGER_USE_STACK
		if (pbuf1 != NULL)
		{
			free(pbuf1);
			pbuf1 = NULL;
		}
#endif
	}

#if defined(JSCUTILS_OS_WINDOWS)
	void Logger::printf(LogType logtype, const wchar_t* format, ...)
	{
		long buf1size = 1536;
#ifdef JSLOGGER_USE_STACK
		wchar_t pbuf1[1536];
#else
		wchar_t *pbuf1 = NULL;
#endif

		va_list args;

		do {
#ifndef JSLOGGER_USE_STACK
			pbuf1 = (wchar_t*)malloc(buf1size);
			if (pbuf1 == NULL)
				break;
#endif

			va_start(args, format);
#ifdef _JSCUTILS_MSVC_CRT_SECURE
			vswprintf_s(pbuf1, buf1size, format, args);
#else
			vswprintf(pbuf1, format, args);
#endif
			va_end(args);

			this->puts(logtype, pbuf1);
		} while (0);

#ifndef JSLOGGER_USE_STACK
		if (pbuf1 != NULL)
		{
			free(pbuf1);
			pbuf1 = NULL;
		}
#endif
	}
#endif
}
