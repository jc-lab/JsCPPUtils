/**
 * @file	Daemon.cpp
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/20
 * @brief	Linux의 Daemon, Windows의 Service application을 구현하기 위한 클래스
 * @warning Singleton class 임
 * @copyright Copyright (C) 2017 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "Common.h"
#include "Daemon.h"

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#if defined(JSCUTILS_OS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#else
#include "StringEncoding.h"
#endif

namespace JsCPPUtils {

	Daemon *Daemon::m_instance = NULL;

	Daemon::Daemon()
	{
		_init();
	}
	
	Daemon::Daemon(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain, const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> &strServiceName)
	{
		_init();

		m_cbparam = cbparam;
		m_daemonstartup = fpdaemonstartup;
		m_daemonmain = fpdaemonmain;
		m_strServiceName = strServiceName;
	}
	
	Daemon *Daemon::createInstance()
	{
		if (m_instance != NULL)
		{
			delete m_instance;
			m_instance = NULL;
		}
		m_instance = new Daemon();
		return m_instance;
	}
	
	Daemon *Daemon::createInstance(void *cbparam, daemonstartup_t fpdaemonstartup, daemonmain_t fpdaemonmain, const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> &strServiceName)
	{
		if (m_instance != NULL)
		{
			delete m_instance;
			m_instance = NULL;
		}
		m_instance = new Daemon(cbparam, fpdaemonstartup, fpdaemonmain, strServiceName);
		return m_instance;
	}
	
	Daemon *Daemon::getInstance()
	{
		return m_instance;
	}

	void Daemon::_init()
	{
		m_plogger = NULL;

		m_sighandler = NULL;
		m_reloadhandler = NULL;
		m_daemonstartup = NULL;
		m_daemonmain = NULL;
		m_helphandler = NULL;
		m_cbparam = NULL;

#if defined(JSCUTILS_OS_WINDOWS)
		memset(&m_ServiceStatus, 0, sizeof(m_ServiceStatus));
		m_StatusHandle = NULL;
		m_fnServiceCtrlHandler = NULL;
		m_dwControlsAccepted = 0;
#endif

		m_isDaemon = false;

		m_bnosetugid = false;
		m_bsetugidforever = false;
		m_args_uid = -1;
		m_args_gid = -1;

		m_runstatus = 0;
	}

	Daemon::~Daemon()
	{
		if(m_plogger != NULL)
		{
			delete m_plogger;
			m_plogger = NULL;
		}
	}

#if defined(JSCUTILS_OS_LINUX)
	int Daemon::main(int argc, _JSCPPUTILS_DAEMON_DEFCHARTYPE *argv[])
	{
		int rc;

		pid_t childpid;
		
		m_main_argc = argc;
		m_main_argv = argv;

		rc = main_parseArgs(argc, argv);
		if (rc != 0)
			return rc;
	
		if (m_isDaemon)
		{
			childpid = fork();
			if (childpid > 0)
			{
				// parent process
				if (this->isArgContain("pidfile"))
				{
					FILE *pidfile_fp = fopen(this->getArg("pidfile").c_str(), "wt");
					if (pidfile_fp == NULL)
					{
						fprintf(stderr, "write pidfile failed: %d\n", errno);
					}
					else {
						fprintf(pidfile_fp, "%d", childpid);
						fclose(pidfile_fp);
					}
				}
				return 0;
			}
			else if (childpid == 0)
			{
				// child process
			}
			else if (childpid == -1)
			{
				// error
				fprintf(stderr, "fork() failed: %d\n", errno);
				return 1;
			}
		}
		
		if (this->isArgContain("logfile"))
		{
			if(m_plogger != NULL)
			{
				delete m_plogger;
				m_plogger = NULL;
			}
			m_plogger = new Logger(Logger::TYPE_FILE, this->getArg("logfile").c_str(), NULL, NULL);
		}
	
		if (this->getArgUid() >= 0)
		{
			FILE *fp_group;
			char strbuf[256];
			int  grouplistcnt = 0;
			gid_t grouplist[64];
		
			fp_group = fopen("/etc/group", "rt");
			if (fp_group == NULL)
			{
				fprintf(stdout, "open /etc/group file failed: %d\n", errno);
				return 1;
			}
		
			while (fgets(strbuf, sizeof(strbuf), fp_group))
			{
				char *pcontext = NULL;
				char *strtoken1 = strtok_r(strbuf, ":", &pcontext);
				char *strtoken2 = strtok_r(NULL, ":", &pcontext);
				char *strtoken3 = strtok_r(NULL, ":", &pcontext);
				char *strtoken4 = strtok_r(NULL, ":", &pcontext);
				char *t_usernames = strtoken4;
				int t_gid = atoi(strtoken3);
				char *t_usernametok;
				size_t tmplen;

				tmplen = strlen(strtoken4);
				while((tmplen > 0) && ((strtoken4[tmplen - 1] == '\r') || (strtoken4[tmplen - 1] == '\n') || (strtoken4[tmplen - 1] == ' ') || (strtoken4[tmplen - 1] == '\t')))
					strtoken4[--tmplen] = 0;

				t_usernametok = strtok_r(t_usernames, ",", &pcontext);
				while (t_usernametok)
				{
					if (strcmp(t_usernametok, this->getArg("user").c_str()) == 0)
					{
						if (grouplistcnt >= 64) break;
						grouplist[grouplistcnt++] = t_gid;
					}
					t_usernametok = strtok_r(NULL, ",", &pcontext);
				}
			}
		
			if (this->isArgContain("logfile") != NULL)
			{
				chown(this->getArg("logfile").c_str(), this->getArgUid(), this->getArgGid());
			}
		
			setgroups(grouplistcnt, grouplist);
		}

		if(m_plogger == NULL)
		{
			m_plogger = new Logger(Logger::TYPE_STDOUT, NULL, NULL, NULL);
		}
	
		if(m_daemonstartup != NULL)
		{
			rc = m_daemonstartup(this, m_cbparam, argc, argv);
			if (rc != 0)
			{
				goto FUNCEXIT;
			}
		}
	
		if ((!m_bnosetugid) && (this->getArgUid() >= 0))
		{
			int arg_uid = this->getArgUid();
			int arg_gid = this->getArgGid();
			if (m_bsetugidforever)
			{
				syscall(SYS_setresgid, arg_gid, arg_gid, arg_gid);
				syscall(SYS_setresuid, arg_uid, arg_uid, arg_uid);
			}
			else
			{
				setgid(arg_gid);
				setuid(arg_uid);
			}
		}
	
		signal(SIGINT, signalhandler);
		signal(SIGTERM, signalhandler);
		signal(SIGHUP, signalhandler);
	
		m_runstatus = 1;
		if(m_daemonmain != NULL)
			rc = m_daemonmain(this, m_cbparam, argc, argv);
		else
			rc = 1;

	FUNCEXIT:
	
		return rc;
	}

	int Daemon::checkArg(const char *arg, const char *prefix)
	{
		char prefixbuf[64];
		strcpy(prefixbuf, prefix);
		strcat(prefixbuf, "=");
		if(strcmp(arg, prefix) == 0)
			return 0;
		else if(strstr(arg, prefixbuf) == arg)
			return strlen(prefixbuf);
		else
			return -1;
	}
	
	int Daemon::findBars(const char *str)
	{
		int nbarcnt = 0;
		size_t tmpsz;
		const char *tmppstr;
					
		tmpsz = strlen(str);
					
		while (((tmpsz - nbarcnt) > 0) && (str[nbarcnt] == '-'))
		{
			nbarcnt++;
		}
		
		return nbarcnt;
	}
	
	int Daemon::main_parseArgs(int argc, char *argv[])
	{
		int rc;

		int argtype;
	
		char *arg_pidfile = NULL;
		int arg_uid = -1;
		int arg_gid = -1;
		char *arg_username = NULL;
		char *arg_logfilepath = NULL;
	
		pid_t childpid;
		
		if (argc > 1)
		{
			int argc2 = argc - 1;
			int index = 1;
			
			m_execname = argv[0];
			
			for (index = 1; argc2 > 0; argc2--, index++)
			{
				if (strcmp(argv[index], "-D") == 0)
				{
					m_isDaemon = true;
					m_args["D"] = "true";
				}
				else if ((argtype = checkArg(argv[index], "--pidfile")) != -1)
				{
					if (argtype == 0)
					{
						arg_pidfile = argv[++index]; argc2--;
					}
					else
					{
						arg_pidfile = &argv[index][argtype];
					}
					m_args["pidfile"] = arg_pidfile;
				}
				else if ((argtype = checkArg(argv[index], "--user")) != -1)
				{
					int block_retval = 0;
					struct passwd pwdbuf;
					size_t buffer_len;
					char *buffer = NULL;
					
					struct passwd *pwd_item = NULL;

					if (argtype == 0)
					{
						arg_username = argv[++index]; argc2--;
					}
					else
					{
						arg_username = &argv[index][argtype];
					}
					
					m_args["user"] = arg_username;

					do
					{
						buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof(char);
						buffer = (char*)malloc(buffer_len);
						if (buffer == NULL)
						{
							fprintf(stderr, "Failed to allocate buffer for getpwnam_r.\n");
							break;
						}
						pwd_item = NULL;
						memset(&pwdbuf, 0, sizeof(pwdbuf));
						getpwnam_r(arg_username, &pwdbuf, buffer, buffer_len, &pwd_item);
						if (pwd_item == NULL)
						{
							fprintf(stderr, "getpwnam_r failed to find requested entry.\n");
							break;
						}
						arg_uid = pwd_item->pw_uid;
						arg_gid = pwd_item->pw_gid;
						m_args_uid = arg_uid;
						m_args_gid = arg_gid;
					} while (0);

					if (buffer) free(buffer);
				
					if (block_retval != 0) return block_retval;
				}
				else if ((argtype = checkArg(argv[index], "--logfile")) != -1)
				{
					if (argtype == 0)
					{
						arg_logfilepath = argv[++index]; argc2--;
					}
					else
					{
						arg_logfilepath = &argv[index][argtype];
					}
					
					m_args["logfile"] = arg_logfilepath;
				}
				else if (strcmp(argv[index], "--help") == 0)
				{
					printhelp();
					return 1;
				}
				else
				{
					int nbarcnt = 0;
					const char *tmppstr;
					const char *tmppstr_val = NULL;
					char tmpname[128];
					
					nbarcnt = findBars(argv[index]);
					
					if (nbarcnt <= 0)
					{
						printhelp();
						return 1;
					}
					
					memset(tmpname, 0, sizeof(tmpname));
					
					tmppstr = &argv[index][nbarcnt];
					tmppstr_val = strstr(tmppstr, "=");
					
					if (tmppstr_val == NULL)
					{
						strcpy(tmpname, tmppstr);
						
						if (argc2 > 1)
						{
							nbarcnt = findBars(argv[index + 1]);
							if (nbarcnt == 0)
							{
								tmppstr_val = argv[++index];
								argc2--;
							}
						}
					}
					else
					{
						strncpy(tmpname, tmppstr, (tmppstr_val - tmppstr));
						tmppstr_val++;
					}
					
					if (tmppstr_val == NULL)
						m_args[tmpname] = "true";
					else
						m_args[tmpname] = tmppstr_val;
				}
			}
		}
	
		return 0;
	}

	void Daemon::signalhandler(int sig)
	{
		int nrst = 0;
		
		if(m_instance == NULL)
			return ;
		
		if(m_instance->m_sighandler != NULL)
			nrst = m_instance->m_sighandler(m_instance, m_instance->m_cbparam, sig);
		
		if (nrst != 0)
			return ;
		
		switch(sig)
		{
		case SIGINT:
		case SIGTERM:
			m_instance->m_runstatus.getifset(2, 1);
			break;
		case SIGHUP:
			if(m_instance->m_reloadhandler != NULL)
				m_instance->m_reloadhandler(m_instance, m_instance->m_cbparam);
			break;
		}
	}

#elif defined(JSCUTILS_OS_WINDOWS)
	void Daemon::cbLoggerToDebugOutput(void *userptr, const char *stroutput)
	{
		::OutputDebugStringA(stroutput);
	}

	int Daemon::main(int argc, _JSCPPUTILS_DAEMON_DEFCHARTYPE *argv[])
	{
		int nrst;

		SERVICE_TABLE_ENTRY ServiceTable[] = 
		{
			{(LPTSTR)m_strServiceName.c_str(), (LPSERVICE_MAIN_FUNCTION) _ServiceMain},
			{NULL, NULL}
		};
		
		m_main_argc = argc;
		m_main_argv = argv;

		nrst = main_parseArgs(argc, argv);
		if (nrst != 0)
			return nrst;

		if (this->isArgContain(_T("logfile")))
		{
			if(m_plogger != NULL)
			{
				delete m_plogger;
				m_plogger = NULL;
			}
			m_plogger = new Logger(Logger::TYPE_FILE, this->getArg(_T("logfile")).c_str(), NULL, NULL);
		}
		if(m_plogger == NULL)
		{
			m_plogger = new Logger(Logger::TYPE_CALLBACK, (const char*)NULL, cbLoggerToDebugOutput, NULL);
		}
		
		if(m_daemonstartup != NULL)
		{
			nrst = m_daemonstartup(this, m_cbparam, argc, argv);
			if(nrst != 0)
				return nrst;
		}

		m_runstatus = 1;

		if(m_isDaemon)
		{
			if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
			{
				OutputDebugString(_T("My Sample Service: Main: StartServiceCtrlDispatcher returned error"));
				return GetLastError();
			}
		}else{
			return m_daemonmain(this, m_cbparam, argc, argv);
		}
		
		return 0;
	}

	int Daemon::checkArg(const TCHAR *arg, const TCHAR *prefix)
	{
		TCHAR prefixbuf[64];
		_tcscpy_s(prefixbuf, prefix);
		_tcscat_s(prefixbuf, _T("="));
		if(_tcscmp(arg, prefix) == 0)
			return 0;
		else if(_tcsstr(arg, prefixbuf) == arg)
			return _tcslen(prefixbuf);
		else
			return -1;
	}
	
	int Daemon::findBars(const TCHAR *str)
	{
		int nbarcnt = 0;
		size_t tmpsz;
		
		tmpsz = _tcslen(str);
					
		while (((tmpsz - nbarcnt) > 0) && (str[nbarcnt] == _T('-')))
		{
			nbarcnt++;
		}
		
		return nbarcnt;
	}
	
	int Daemon::main_parseArgs(int argc, TCHAR *argv[])
	{
		int argtype;
	
		TCHAR *arg_pidfile = NULL;
		TCHAR *arg_logfilepath = NULL;
	
		if (argc > 1)
		{
			int argc2 = argc - 1;
			int index = 1;
			
			m_execname = argv[0];
			
			for (index = 1; argc2; argc2--, index++)
			{
				if (_tcscmp(argv[index], _T("-D")) == 0)
				{
					m_isDaemon = true;
					m_args[_T("D")] = _T("true");
				}
				else if ((argtype = checkArg(argv[index], _T("--pidfile"))) != -1)
				{
					if (argtype == 0)
					{
						arg_pidfile = argv[++index]; argc2--;
					}
					else
					{
						arg_pidfile = &argv[index][argtype];
					}
					m_args[_T("pidfile")] = arg_pidfile;
				}
				else if ((argtype = checkArg(argv[index], _T("--logfile"))) != -1)
				{
					if (argtype == 0)
					{
						arg_logfilepath = argv[++index]; argc2--;
					}
					else
					{
						arg_logfilepath = &argv[index][argtype];
					}
					
					m_args[_T("logfile")] = arg_logfilepath;
				}
				else if (_tcscmp(argv[index], _T("--help")) == 0)
				{
					printhelp();
					return 1;
				}
				else
				{
					int nbarcnt = 0;
					const TCHAR *tmppstr;
					const TCHAR *tmppstr_val = NULL;
					TCHAR tmpname[128];
					
					nbarcnt = findBars(argv[index]);
					
					if (nbarcnt <= 0)
					{
						printhelp();
						return 1;
					}
					
					memset(tmpname, 0, sizeof(tmpname));
					
					tmppstr = &argv[index][nbarcnt];
					tmppstr_val = _tcsstr(tmppstr, _T("="));
					
					if (tmppstr_val == NULL)
					{
						_tcscpy_s(tmpname, tmppstr);
						
						if (argc2 > 1)
						{
							nbarcnt = findBars(argv[index + 1]);
							if (nbarcnt == 0)
							{
								tmppstr_val = argv[++index];
								argc2--;
							}
						}
					}
					else
					{
						_tcsncpy_s(tmpname, tmppstr, (tmppstr_val - tmppstr));
						tmppstr_val++;
					}
					
					if (tmppstr_val == NULL)
						m_args[tmpname] = _T("true");
					else
						m_args[tmpname] = tmppstr_val;
					
					argc2--;
				}
			}
		}
	
		return 0;
	}

	void Daemon::setControlsAccepted(DWORD dwControlsAccepted)
	{
		m_dwControlsAccepted = dwControlsAccepted;
	}
	
	void Daemon::setServiceCtrlHandler(fnServiceCtrlHandler_t fnHandler)
	{
		m_fnServiceCtrlHandler = fnHandler;
	}

	void Daemon::sysDebugPrintf(const char *format, ...)
	{
		char szBuffer[1024];
		va_list vargs;
		va_start(vargs, format);
		vsprintf_s(szBuffer, format, vargs);
		va_end(vargs);
		::OutputDebugStringA(szBuffer);
	}

	void Daemon::sysDebugPrintf(const wchar_t *format, ...)
	{
		wchar_t szBuffer[1024];
		va_list vargs;
		va_start(vargs, format);
		vswprintf_s(szBuffer, format, vargs);
		va_end(vargs);
		::OutputDebugStringW(szBuffer);
	}

	VOID Daemon::_ServiceMain(DWORD argc, LPTSTR *argv)
	{
		DWORD Status = E_FAIL;
		HANDLE hThread;

		OutputDebugString(_T("My Sample Service: ServiceMain: Entry"));

		m_instance->m_StatusHandle = RegisterServiceCtrlHandlerEx(m_instance->m_strServiceName.c_str(), _ServiceCtrlHandlerEx, m_instance);

		if (m_instance->m_StatusHandle == NULL) 
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error"));
			goto EXIT;
		}
        
		// Tell the service controller we are starting
		memset(&m_instance->m_ServiceStatus, 0, sizeof(m_instance->m_ServiceStatus));
		m_instance->m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		m_instance->m_ServiceStatus.dwControlsAccepted = 0;
		m_instance->m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
		m_instance->m_ServiceStatus.dwWin32ExitCode = 0;
		m_instance->m_ServiceStatus.dwServiceSpecificExitCode = 0;
		m_instance->m_ServiceStatus.dwCheckPoint = 0;

		if (SetServiceStatus(m_instance->m_StatusHandle, &m_instance->m_ServiceStatus) == FALSE) 
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		/* 
		 * Perform tasks neccesary to start the service here
		 */
		OutputDebugString(_T("My Sample Service: ServiceMain: Performing Service Start Operations"));

		// Create stop event to wait on later.
		/*
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

			m_instance->m_ServiceStatus.dwControlsAccepted = 0;
			m_instance->m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			m_instance->m_ServiceStatus.dwWin32ExitCode = GetLastError();
			m_instance->m_ServiceStatus.dwCheckPoint = 1;

			if (SetServiceStatus(m_instance->m_StatusHandle, &m_instance->m_ServiceStatus) == FALSE)
			{
				OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
			}
			goto EXIT; 
		}
		*/

		// Tell the service controller we are started
		m_instance->m_ServiceStatus.dwControlsAccepted = m_instance->m_dwControlsAccepted | SERVICE_ACCEPT_STOP;
		m_instance->m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		m_instance->m_ServiceStatus.dwWin32ExitCode = 0;
		m_instance->m_ServiceStatus.dwCheckPoint = 0;

		if (SetServiceStatus(m_instance->m_StatusHandle, &m_instance->m_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		hThread = CreateThread(NULL, 0, _ServiceWorkerThread, Daemon::m_instance, 0, NULL);

		OutputDebugString(_T("My Sample Service: ServiceMain: Waiting for Worker Thread to complete"));

		// Wait until our worker thread exits effectively signaling that the service needs to stop
		WaitForSingleObject (hThread, INFINITE);
    
		OutputDebugString(_T("My Sample Service: ServiceMain: Worker Thread Stop Event signaled"));
    
		/* 
		 * Perform any cleanup tasks
		 */
		OutputDebugString(_T("My Sample Service: ServiceMain: Performing Cleanup Operations"));

		CloseHandle (hThread);

		m_instance->m_ServiceStatus.dwControlsAccepted = 0;
		m_instance->m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		m_instance->m_ServiceStatus.dwWin32ExitCode = 0;
		m_instance->m_ServiceStatus.dwCheckPoint = 3;

		if (SetServiceStatus(m_instance->m_StatusHandle, &m_instance->m_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}
    
		EXIT:
		OutputDebugString(_T("My Sample Service: ServiceMain: Exit"));

		return;
	}
	
	DWORD WINAPI Daemon::_ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
	{
		Daemon *pDaemon = (Daemon*)lpContext;
		OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: Entry"));

		switch (dwControl) 
		{
		 case SERVICE_CONTROL_STOP :

			OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

			if (pDaemon->m_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			   break;

			pDaemon->m_ServiceStatus.dwControlsAccepted = 0;
			pDaemon->m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			pDaemon->m_ServiceStatus.dwWin32ExitCode = 0;
			pDaemon->m_ServiceStatus.dwCheckPoint = 4;

			if(SetServiceStatus(pDaemon->m_StatusHandle, &pDaemon->m_ServiceStatus) == FALSE)
			{
				OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
			}

			// This will signal the worker thread to start shutting down
			pDaemon->m_runstatus.getifset(2, 1);

			break;

		 default:
			 break;
		}

		if(pDaemon->m_fnServiceCtrlHandler != NULL)
		{
			pDaemon->m_fnServiceCtrlHandler(pDaemon, dwControl, dwEventType, lpEventData);
		}

		OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: Exit"));

		return NO_ERROR;
	}

	DWORD WINAPI Daemon::_ServiceWorkerThread(LPVOID lpParam)
	{
		Daemon *pthis = (Daemon*)lpParam;
		return (DWORD)pthis->m_daemonmain(pthis, pthis->m_cbparam, pthis->m_main_argc, pthis->m_main_argv);
	}

#endif
	
	void Daemon::printhelp()
	{
		printf("%s\n", m_execname.c_str());
		printf("\t-D : Daemon\n");
		printf("\t--pidfile FILEPATH\n");
		printf("\t--user USERNAME\n");
		printf("\t--logfile LOGFILE\n");
		printf("\t--help\n");
		if (m_helphandler != NULL)
			m_helphandler(this, m_cbparam);
	}
		
	bool Daemon::isDaemon()
	{
		return m_isDaemon;
	}

	int Daemon::getRunStatus()
	{
		return m_runstatus.get();
	}

	void Daemon::reqStop()
	{
		m_runstatus.getifset(2, 1);
	}
		
	void Daemon::setNoSetUGID(bool bvalue)
	{
		m_bnosetugid = bvalue;
	}
	
	void Daemon::setSetUGIDForever(bool bvalue)
	{
		m_bsetugidforever = bvalue;
	}
		
	void Daemon::setCallbackParam(void *cbparam)
	{
		m_cbparam = cbparam;
	}

	void Daemon::setSigHandler(sighandler_t fphandler)
	{
		m_sighandler = fphandler;
	}

	void Daemon::setReloadHandler(reloadhandler_t fphandler)
	{
		m_reloadhandler = fphandler;
	}
	
	void Daemon::setDaemonStartup(daemonstartup_t fpdaemonstartup)
	{
		m_daemonstartup = fpdaemonstartup;
	}
	
	void Daemon::setDaemonMain(daemonmain_t fpdaemonmain)
	{
		m_daemonmain = fpdaemonmain;
	}
	
	void Daemon::setHelpHandler(helphandler_t fphandler)
	{
		m_helphandler = fphandler;
	}

	Logger *Daemon::getLogger()
	{
		return m_plogger;
	}
	
	bool Daemon::isArgContain(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *szName)
	{
		std::map<std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>, std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> >::iterator iter = m_args.find(szName);
		return (iter != m_args.end());
	}
	
	bool Daemon::isArgContain(const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& strName)
	{
		std::map<std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>, std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> >::iterator iter = m_args.find(strName);
		return (iter != m_args.end());
	}
	
	const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& Daemon::getArg(const _JSCPPUTILS_DAEMON_DEFCHARTYPE *szName)
	{
		std::map<std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>, std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> >::iterator iter = m_args.find(szName);
		if (iter != m_args.end())
		{
			return iter->second;
		}
		else
		{
			return m_arg_nullstr;
		}
	}
	
	const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& Daemon::getArg(const std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>& strName)
	{
		std::map<std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE>, std::basic_string<_JSCPPUTILS_DAEMON_DEFCHARTYPE> >::iterator iter = m_args.find(strName);
		if (iter != m_args.end())
		{
			return iter->second;
		}
		else
		{
			return m_arg_nullstr;
		}
	}
		
	int Daemon::getArgUid()
	{
		return m_args_uid;
	}
	
	int Daemon::getArgGid()
	{
		return m_args_gid;
	}
}
