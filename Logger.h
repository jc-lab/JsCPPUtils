/**
 * @file	Logger.cpp
 * @class	Logger
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils)
 * @date	2016/11/02
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_LOGGER_H__
#define __JSCPPUTILS_LOGGER_H__

#include <string>
#include <stdio.h>

#include "Common.h"
#include "Lockable.h"
#include "SmartPointer.h"

#define JSLOGGER_USE_STACK

namespace JsCPPUtils
{
	class Logger : public Lockable
	{
	public:
		enum OutputType {
			TYPE_NULL = 0,
			TYPE_STDOUT,
			TYPE_STDERR,
			TYPE_FILE,
			TYPE_CALLBACK,
			TYPE_SYSLOG
		};

		enum LogType {
			LOGTYPE_EMERG = 0,
			LOGTYPE_ALERT = 1,
			LOGTYPE_CRIT = 2,
			LOGTYPE_ERR = 3,
			LOGTYPE_WARNING = 4,
			LOGTYPE_NOTICE = 5,
			LOGTYPE_INFO = 6,
			LOGTYPE_DEBUG = 7
		};

		typedef void(*CallbackFuncA_t)(void *userptr, const char *stroutput);
		typedef void(*CallbackFuncW_t)(void *userptr, const wchar_t *stroutput);

	private:
		Logger *m_pParent;
		JsCPPUtils::SmartPointer<Logger> m_spParent;
		std::string m_strPrefixName;

		FILE *m_fp;
		OutputType m_outtype;

		CallbackFuncA_t m_cbfuncA;
		CallbackFuncW_t m_cbfuncW;
		void *m_cbuserptr;

		int m_lasterrno;

		void _clearInstance();

		void _child_puts(LogType logtype, const std::string& strPrefixName, const char* szLog);
		void _child_puts(LogType logtype, const std::string& strPrefixName, const wchar_t* szLog);

	public:
		Logger();
		int init(OutputType outputType, const char *szFilePath, CallbackFuncA_t cbfuncA = NULL, CallbackFuncW_t cbfuncW = NULL, void *cbuserptr = NULL);
		int init(Logger *pParent, const std::string& strPrefixName);
		int init(const JsCPPUtils::SmartPointer<Logger> &spParent, const std::string& strPrefixName);
		Logger(OutputType outputType, const char *szFilePath, CallbackFuncA_t cbfuncA = NULL, CallbackFuncW_t cbfuncW = NULL, void *cbuserptr = NULL);
		Logger(Logger *pParent, const std::string& strPrefixName);
		Logger(const JsCPPUtils::SmartPointer<Logger> &spParent, const std::string& strPrefixName);
#if defined(JSCUTILS_OS_WINDOWS)
		int init(OutputType outputType, const wchar_t *szFilePath, CallbackFuncA_t cbfuncA = NULL, CallbackFuncW_t cbfuncW = NULL, void *cbuserptr = NULL);
		Logger(OutputType outputType, const wchar_t *szFilePath, CallbackFuncA_t cbfuncA = NULL, CallbackFuncW_t cbfuncW = NULL, void *cbuserptr = NULL);
#endif
		void close();
		~Logger();
		int getErrno() {
			return m_lasterrno;
		};
		void setParent(Logger *pParent);
		void setParent(const JsCPPUtils::SmartPointer<Logger> &spParent);
		void printf(LogType logtype, const char* format, ...);
		void puts(LogType logtype, const char* text);
#if defined(JSCUTILS_OS_WINDOWS)
		void printf(LogType logtype, const wchar_t* format, ...);
		void puts(LogType logtype, const wchar_t* text);
#endif
	};

}

#endif /* __JSCPPUTILS_LOGGER_H__ */
