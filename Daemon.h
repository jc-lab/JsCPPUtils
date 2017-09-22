/**
 * @file	Daemon.h
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/20
 * @copyright Copyright (C) 2017 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_DAEMON_H__
#define __JSCPPUTILS_DAEMON_H__

#include "Common.h"
#include "AtomicNum.h"
#include "Logger.h"

#if defined(JSCUTILS_OS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#define _JSCPPUTILS_DAEMON_DEFCHARTYPE char
#define _JSCPPUTILS_DAEMON_DEFCHARTYPE_T(T) T

#elif defined(JSCUTILS_OS_WINDOWS)
#include <windows.h>
#include <tchar.h>

#define _JSCPPUTILS_DAEMON_DEFCHARTYPE TCHAR
#define _JSCPPUTILS_DAEMON_DEFCHARTYPE_T(T) _T(T)

#endif

#include <map>
#include <string>

namespace JsCPPUtils
{

	class Daemon
	{
	public:
		typedef int (*sighandler_t)(Daemon *pdaemon, void *cbparam, int sig);
		typedef void (*reloadhandler_t)(Daemon *pdaemon, void *cbparam);
#if defined(JSCUTILS_OS_LINUX)
		typedef int (*daemonstartup_t)(Daemon *pdaemon, void *cbparam, int argc, char *argv[]);
		typedef int (*daemonmain_t)(Daemon *pdaemon, void *cbparam, int argc, char *argv[]);
#elif defined(JSCUTILS_OS_WINDOWS)
		typedef int (*daemonstartup_t)(Daemon *pdaemon, void *cbparam, int argc, TCHAR *argv[]);
		typedef int (*daemonmain_t)(Daemon *pdaemon, void *cbparam, int argc, TCHAR *argv[]);

		typedef void (*fnServiceCtrlHandler_t)(Daemon *pDaemon, DWORD dwControl, DWORD dwEventType, LPVOID lpEventData);

#endif
		typedef void (*helphandler_t)(Daemon *pdaemon, void *cbparam);

	private:
		static Daemon *m_instance;
		
		std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> m_strServiceName;

#if defined(JSCUTILS_OS_WINDOWS)
		SERVICE_STATUS        m_ServiceStatus;
		SERVICE_STATUS_HANDLE m_StatusHandle;

		fnServiceCtrlHandler_t m_fnServiceCtrlHandler;
#endif

		/**
		 * 0 : stopped
		 * 1 : running
		 * 2 : stopping
		 */
		AtomicNum<int> m_runstatus;

		Logger *m_plogger;

		std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> m_execname;
		std::map<std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>, std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> > m_args;
		std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> m_arg_nullstr;
		
		void *m_cbparam;
		sighandler_t m_sighandler;
		reloadhandler_t m_reloadhandler;
		daemonstartup_t m_daemonstartup;
		daemonmain_t m_daemonmain;
		helphandler_t m_helphandler;
		
		int m_main_argc;
		_JSCPPUTILS_DAEMON_DEFCHARTYPE **m_main_argv;

		bool m_isDaemon;
		
		bool m_bnosetugid;
		bool m_bsetugidforever;
		int m_args_uid;
		int m_args_gid;

		int checkArg(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *arg, const _JSCPPUTILS_DAEMON_DEFCHARTYPE *prefix);
		
		static int findBars(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *str);

	private:
		Daemon();
		Daemon(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain, const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> &strServiceName = _JSCPPUTILS_DAEMON_DEFCHARTYPE_T(""));
		
#if defined(JSCUTILS_OS_WINDOWS)
		static VOID WINAPI _ServiceMain(DWORD argc, LPTSTR *argv);
		static DWORD WINAPI _ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
		static DWORD WINAPI _ServiceWorkerThread(LPVOID lpParam);
#endif
		
	public:
		~Daemon();

		static Daemon *createInstance();
		static Daemon *createInstance(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain, const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> &strServiceName = _JSCPPUTILS_DAEMON_DEFCHARTYPE_T(""));
		static Daemon *getInstance();
		
		void _init();

		static void signalhandler(int sig);

		int main(int argc, _JSCPPUTILS_DAEMON_DEFCHARTYPE *argv[]);
		int main_parseArgs(int argc, _JSCPPUTILS_DAEMON_DEFCHARTYPE *argv[]);
		
		bool isDaemon();

		int getRunStatus();
		
		void printhelp();
		
		void setNoSetUGID(bool bvalue = true);
		void setSetUGIDForever(bool bvalue = true);
		
		void setCallbackParam(void *cbparam);
		void setSigHandler(sighandler_t fphandler);
		void setReloadHandler(reloadhandler_t fphandler);
		void setDaemonStartup(daemonstartup_t fpdaemonstartup);
		void setDaemonMain(daemonmain_t fpdaemonmain);
		void setHelpHandler(helphandler_t fphandler);

		Logger *getLogger();
		
		bool isArgContain(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *szName);
		bool isArgContain(const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& strName);
		const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& getArg(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *szName);
		const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& getArg(const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& strName);
		
		int getArgUid();
		int getArgGid();

		
#if defined(JSCUTILS_OS_WINDOWS)
		static void sysDebugPrintf(const char *format, ...);
		static void sysDebugPrintf(const wchar_t *format, ...);
		static void cbLoggerToDebugOutput(void *userptr, const char *stroutput);
		void setServiceCtrlHandler(fnServiceCtrlHandler_t fnHandler);
#endif
	};

}

#endif /* __JSCPPUTILS_DAEMON_H__ */
