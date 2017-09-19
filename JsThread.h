/**
 * @file	JsThread.h
 * @class	JsThread
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/11/03
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_JSTHREAD_H__
#define __JSCPPUTILS_JSTHREAD_H__

#include "Common.h"

#include <stdlib.h>
#include <string.h>

#if defined(JSCUTILS_OS_LINUX)
#include <pthread.h>

#define JSTHREAD_THREADID_TYPE pthread_t
#elif defined(JSCUTILS_OS_WINDOWS)
#include <Windows.h>

#define JSTHREAD_THREADID_TYPE DWORD
#endif

#include "SmartPointer.h"
#include "AtomicNum.h"

namespace JsCPPUtils
{
	class JsThread
	{
	public:
		class ThreadContext;

		typedef int (*StartRoutine_t)(ThreadContext *pThreadCtx, int param_idx, void *param_ptr);

		enum RunningStatus
		{
			RS_INITALIZING = 0,
			RS_INITALIZED = 1,
			RS_STARTED = 2,
			RS_STOPPED = 3
		};
		
		class ThreadContext {
		friend class JsThread;
		private:
			int m_index;
			
			JSTHREAD_THREADID_TYPE m_tid;

#if defined(JSCUTILS_OS_LINUX)
			pthread_t m_pthread;
			pthread_mutex_t m_run_mutex;
#elif defined(JSCUTILS_OS_WINDOWS)
			HANDLE m_hThread;
			HANDLE m_stop_hEvent;
#endif
			
			StartRoutine_t m_startroutine;
			void *m_param;
			
			JsCPPUtils::AtomicNum<int> m_runningstatus;

			ThreadContext(StartRoutine_t startroutine, int index, void *param)
				: m_startroutine(startroutine)
				, m_index(index)
				, m_param(param)
				, m_runningstatus(0)
			{
#if defined(JSCUTILS_OS_LINUX)
			 m_pthread = 0;
			 memset(&m_run_mutex, 0, sizeof(m_run_mutex));
#elif defined(JSCUTILS_OS_WINDOWS)
			m_hThread = INVALID_HANDLE_VALUE;;
			m_stop_hEvent = INVALID_HANDLE_VALUE;
#endif
			}
		public:
			int reqStop();
			int _inthread_isRun();
			RunningStatus getRunningStatus();
			JSTHREAD_THREADID_TYPE getThreadId();
			int join();

			~ThreadContext()
			{
				if((m_runningstatus.get() > 0) && (m_runningstatus.get() < 3))
				{
					reqStop();
					join();
				}
#if defined(JSCUTILS_OS_LINUX)
				::pthread_mutex_destroy(&m_run_mutex);
#elif defined(JSCUTILS_OS_WINDOWS)
				if((m_stop_hEvent != NULL) && (m_stop_hEvent != INVALID_HANDLE_VALUE))
				{
					::CloseHandle(m_stop_hEvent);
					m_stop_hEvent = NULL;
				}
				if((m_hThread != NULL) && (m_hThread != INVALID_HANDLE_VALUE))
				{
					::CloseHandle(m_hThread);
					m_hThread = NULL;
				}
#endif
			}
		};
		
		// This is not async-signal safe
#if defined(JSCUTILS_OS_LINUX)
		template<class T>
		class MessageHandler
		{
		private:
			pthread_mutex_t m_mutex;
			pthread_cond_t m_cond_send;
			pthread_cond_t m_cond_ack;
			JsCPPUtils::SmartPointer<T> m_spmsg;
			volatile int m_status;
			
		public:
			MessageHandler()
			{
				pthread_mutex_init(&m_mutex, NULL);
				pthread_cond_init(&m_cond_send, NULL);
				pthread_cond_init(&m_cond_ack, NULL);
				m_status = 0;
			}
	
			int post(JsCPPUtils::SmartPointer<T> spmsg, bool isnonblock = false)
			{
				pthread_mutex_lock(&m_mutex);
				m_spmsg = spmsg;
				m_status = 1;
				pthread_cond_signal(&m_cond_send);
				if (!isnonblock)
					pthread_cond_wait(&m_cond_ack, &m_mutex);
				pthread_mutex_unlock(&m_mutex);
				return 1;
			}
	
			int postCancel()
			{
				pthread_mutex_lock(&m_mutex);
				m_status = 2;
				pthread_cond_signal(&m_cond_send);
				pthread_mutex_unlock(&m_mutex);
				return 1;
			}
	
			int getmsg(JsCPPUtils::SmartPointer<T> *pspmsg, long timeoutms = -1)
			{
				int retval = 0;
		
				pthread_mutex_lock(&m_mutex);
				if (timeoutms == 0)
				{
					if (m_status == 1)
						retval = 1;
					if (m_status != 0)
						pthread_cond_signal(&m_cond_ack); // Need check it!!!
				}
				else if (timeoutms < 0)
				{
					while (m_status == 0)
						pthread_cond_wait(&m_cond_send, &m_mutex);
					pthread_cond_signal(&m_cond_ack);
					if (m_status == 1)
						retval = 1;
				}
				else
				{
					struct timespec ts;
					ts.tv_sec = timeoutms / 1000;
					ts.tv_nsec = ((long long)timeoutms % 1000) * 1000000;
					while (m_status == 0)
						pthread_cond_timedwait(&m_cond_send, &m_mutex, &ts);
					pthread_cond_signal(&m_cond_ack);
					if (m_status == 1)
						retval = 1;
				}
		
				if ((pspmsg != NULL) && (m_status == 1))
					(*pspmsg) = m_spmsg;
				m_spmsg = NULL;
				
				m_status = 0;
		
				pthread_mutex_unlock(&m_mutex);
		
				return retval;
			}
			
			int handlemsg_begin(JsCPPUtils::SmartPointer<T> *pspmsg, long timeoutms = -1)
			{
				int retval = 0;
		
				pthread_mutex_lock(&m_mutex);
				if (timeoutms == 0)
				{
					if (m_status == 1)
						retval = 1;
					if (m_status != 0)
						pthread_cond_signal(&m_cond_send);
				}
				else if (timeoutms < 0)
				{
					while (m_status == 0)
						pthread_cond_wait(&m_cond_send, &m_mutex);
					pthread_cond_signal(&m_cond_ack);
					if (m_status == 1)
						retval = 1;
				}
				else
				{
					struct timespec ts;
					ts.tv_sec = timeoutms / 1000;
					ts.tv_nsec = ((long long)timeoutms % 1000) * 1000000;
					while (m_status == 0)
						pthread_cond_timedwait(&m_cond_send, &m_mutex, &ts);
					pthread_cond_signal(&m_cond_ack);
					if (m_status == 1)
						retval = 1;
				}
		
				if ((pspmsg != NULL) && (m_status == 1))
					(*pspmsg) = m_spmsg;
		
				if (retval != 1)
				{
					m_status = 0;
					pthread_mutex_unlock(&m_mutex);
				}
		
				return retval;
			}
			
			void handlemsg_end()
			{
				if (m_status == 1)
				{
					m_spmsg = NULL;
					m_status = 0;
					pthread_mutex_unlock(&m_mutex);
				}
			}
		};
#elif defined(JSCUTILS_OS_WINDOWS)
	template<class T>
		class MessageHandler
		{
		private:
			HANDLE m_mutex;
			HANDLE m_cond_send;
			HANDLE m_cond_ack;
			JsCPPUtils::SmartPointer<T> m_spmsg;
			volatile int m_status;
			
		public:
			MessageHandler()
			{
				m_mutex = ::CreateMutex(NULL, FALSE, NULL);
				m_cond_send = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				m_cond_ack = ::CreateEvent(NULL, FALSE, FALSE, NULL);
				m_status = 0;
			}
			~MessageHandler()
			{
				if(m_mutex != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(m_mutex);
					m_mutex = INVALID_HANDLE_VALUE;
				}
				if(m_cond_send != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(m_cond_send);
					m_cond_send = INVALID_HANDLE_VALUE;
				}
				if(m_cond_ack != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(m_cond_ack);
					m_cond_ack = INVALID_HANDLE_VALUE;
				}
			}
	
			int post(JsCPPUtils::SmartPointer<T> spmsg, long timeoutms = -1)
			{
				int retval = 0;
				DWORD dwWait;
				bool bLocked = false;

				do {
					dwWait = ::WaitForSingleObject(m_mutex, (timeoutms < 0) ? INFINITE : timeoutms); // 다른 쓰레드에서 Post중이라면
					if(dwWait == WAIT_TIMEOUT)
					{
						retval = 3; // 다른 쓰레드에서 post작업이 끝나지 않았습니다.
						break;
					}else if(dwWait != WAIT_OBJECT_0)
					{
						retval = -((int)dwWait);
						break;
					}
					bLocked = true;

					if(m_status != 0)
					{
						dwWait = ::WaitForSingleObject(m_cond_ack, (timeoutms < 0) ? INFINITE : timeoutms);
						if(dwWait == WAIT_TIMEOUT)
						{
							retval = 3; // 이전 작업이 아직 끝나지 않았습니다.
							break;
						}else if(dwWait != WAIT_OBJECT_0)
						{
							retval = -((int)dwWait);
							break;
						}
					}

					::ResetEvent(m_cond_ack);
					m_spmsg = spmsg;
					m_status = 1;
					::SetEvent(m_cond_send);

					dwWait = WAIT_OBJECT_0;
					if (timeoutms != 0)
						dwWait = ::WaitForSingleObject(m_cond_ack, (timeoutms < 0) ? INFINITE : timeoutms);
					
					switch(dwWait)
					{
					case WAIT_OBJECT_0:
						retval = 1;
						break;
					case WAIT_TIMEOUT:
						retval = 0;
						break;
					default:
						retval = -((int)dwWait);
						break;
					}

				}while(0);

				if(bLocked)
				{
					::ReleaseMutex(m_mutex);
				}

				return retval;
			}
	
			int postCancel()
			{
				::WaitForSingleObject(m_mutex, INFINITE);
				m_spmsg = NULL;
				m_status = 2;
				::SetEvent(m_cond_send);
				::ReleaseMutex(m_mutex);
				return 1;
			}
	
			int getmsg(JsCPPUtils::SmartPointer<T> *pspmsg, long timeoutms = -1, bool bProcessedEvent = false)
			{
				int retval = 0;
				DWORD dwWait;

				if(bProcessedEvent)
				{
					dwWait = WAIT_OBJECT_0;
				}else{
					dwWait = ::WaitForSingleObject(m_cond_send, (timeoutms < 0) ? INFINITE : timeoutms);
				}
				switch(dwWait)
				{
				case WAIT_OBJECT_0:
					if ((pspmsg != NULL) && (m_status == 1))
						(*pspmsg) = m_spmsg;
					m_spmsg = NULL;
					::SetEvent(m_cond_ack);
					break;
				case WAIT_TIMEOUT:
					retval = 0;
					break;
				default:
					retval = -((int)dwWait);
					break;
				}
				m_status = 0;
		
				return retval;
			}

			HANDLE getEventHandle()
			{
				return m_cond_send;
			}
			
			int handlemsg_begin(JsCPPUtils::SmartPointer<T> *pspmsg, long timeoutms = -1, bool bProcessedEvent = false)
			{
				int retval = 0;
				DWORD dwWait;

				if(bProcessedEvent)
				{
					dwWait = WAIT_OBJECT_0;
				}else{
					dwWait = ::WaitForSingleObject(m_cond_send, (timeoutms < 0) ? INFINITE : timeoutms);
				}
				switch(dwWait)
				{
				case WAIT_OBJECT_0:
					if ((pspmsg != NULL) && (m_status == 1))
						(*pspmsg) = m_spmsg;
					m_spmsg = NULL;
					if(m_status == 1)
					{
						retval = 1;
					}else{
						m_status = 0;
						::SetEvent(m_cond_ack);
					}
					break;
				case WAIT_TIMEOUT:
					retval = 0;
					break;
				default:
					retval = -((int)dwWait);
					break;
				}
		
				return retval;
			}
			
			void handlemsg_end()
			{
				if (m_status == 1)
				{
					m_spmsg = NULL;
					m_status = 0;
					::SetEvent(m_cond_ack);
				}
			}
		};
#endif

	private:
#if defined(JSCUTILS_OS_LINUX)
		static void *threadProc(void *param);
#elif defined(JSCUTILS_OS_WINDOWS)
		static DWORD WINAPI threadProc(LPVOID param);
#endif
			
	public:
		static int start(JsCPPUtils::SmartPointer<ThreadContext> *pspThreadCtx, StartRoutine_t startroutine, int param_idx, void *param_ptr, JSTHREAD_THREADID_TYPE *pThreadId = NULL, const char *szThreadName = NULL);
		static int reqStop(ThreadContext *pThreadCtx);
	};
}

#endif /* __JSCPPUTILS_JSTHREAD_H__ */
