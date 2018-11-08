/**
 * @file	thread.cpp
 * @class	thread
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/10/08
 * @brief	JsThread
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <errno.h>

#include "thread.h"

namespace JsCPPUtils
{
	Thread::Thread()
	{
#if defined(JSCUTILS_OS_LINUX)
		m_pthread = 0;
		memset(&m_run_mutex, 0, sizeof(m_run_mutex));
#elif defined(JSCUTILS_OS_WINDOWS)
		m_hThread = INVALID_HANDLE_VALUE;;
		m_stop_hEvent = INVALID_HANDLE_VALUE;
#endif
		m_retval = 0;
	}
	
	Thread::~Thread()
	{
		if ((m_runningstatus.get() > 0) && (m_runningstatus.get() < 3))
		{
			reqStop();
			join();
		}
#if defined(JSCUTILS_OS_LINUX)
		::pthread_mutex_destroy(&m_run_mutex);
#elif defined(JSCUTILS_OS_WINDOWS)
		if ((m_stop_hEvent != NULL) && (m_stop_hEvent != INVALID_HANDLE_VALUE))
		{
			::CloseHandle(m_stop_hEvent);
			m_stop_hEvent = NULL;
		}
		if ((m_hThread != NULL) && (m_hThread != INVALID_HANDLE_VALUE))
		{
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
#endif
	}

#if defined(JSCUTILS_OS_LINUX)
	void *Thread::threadProcV2(void *param)
	{
		JsCPPUtils::SmartPointer<Thread> spThread;
		int retval;
		spThread.attach((JsCPPUtils::SmartPointer<Thread>*)param);

		spThread->m_runningstatus = 2;
		retval = spThread->run(spThread->m_index, spThread->m_param);
		spThread->m_runningstatus = 3;

		return (DWORD)retval;
	}
#elif defined(JSCUTILS_OS_WINDOWS)
	DWORD WINAPI Thread::threadProcV2(LPVOID param)
	{
		JsCPPUtils::SmartPointer<Thread> spThread;
		int retval;
		spThread.attach((JsCPPUtils::SmartPointer<Thread>*)param);

		spThread->m_runningstatus = 2;
		spThread->m_retval = retval = spThread->run(spThread->m_index, spThread->m_param);
		spThread->m_runningstatus = 3;

		return (DWORD)retval;
	}
#endif

	int Thread::reqStop()
	{
#if defined(JSCUTILS_OS_LINUX)
		int nrst = pthread_mutex_unlock(&m_run_mutex);
		pthread_cancel(m_pthread);
		if(nrst != 0)
			return -errno;
		return 1;
#elif defined(JSCUTILS_OS_WINDOWS)
		if(m_stop_hEvent == INVALID_HANDLE_VALUE)
			return 0;
		if(!SetEvent(m_stop_hEvent))
		{
			return -((int)GetLastError());
		}
		return 1;
#endif
	}

	int Thread::isRunning()
	{
#if defined(JSCUTILS_OS_LINUX)
		int nrst = pthread_mutex_trylock(&m_run_mutex);
		switch(nrst)
		{
		case 0: // unlocked
			pthread_mutex_unlock(&m_run_mutex);
			return 0; // quit
		case EBUSY: // Already locked
			return 1; // run
		}
		return -nrst; // error / quit
#elif defined(JSCUTILS_OS_WINDOWS)
		DWORD dwWait = ::WaitForSingleObject(m_stop_hEvent, 0);
		switch(dwWait)
		{
		case WAIT_OBJECT_0:
			return 0; // quit
		case WAIT_TIMEOUT:
			return 1; //run
		}
		return -((int)dwWait);
#endif
	}
	
	Thread::RunningStatus Thread::getRunningStatus()
	{
		return (RunningStatus)m_runningstatus.get();
	}

#if defined(JSCUTILS_OS_WINDOWS)
	DWORD Thread::getThreadId()
	{
		return m_tid;
	}
#elif defined(JSCUTILS_OS_LINUX)
	pthread_t Thread::getThreadId()
	{
		return m_tid;
	}
#endif

#if defined(JSCUTILS_OS_WINDOWS)
	int Thread::join(DWORD dwTimeout)
	{
		int64_t st = JsCPPUtils::Common::getTickCount();
		if ((m_stop_hEvent == NULL) || (m_stop_hEvent == INVALID_HANDLE_VALUE))
			return 0;
		//::WaitForSingleObject(m_stop_hEvent, dwTimeout);

		while (m_runningstatus.get() != 3)
		{
			int64_t ct = JsCPPUtils::Common::getTickCount();
			int64_t dt = ct - st;
			if ((dwTimeout != INFINITE) && (dt >= dwTimeout))
				break;
			::Sleep(10);
		}

		return 1;
	}
#elif defined(JSCUTILS_OS_LINUX)
	int JsThread::ThreadContext::join(int nTimeout)
	{
		void *pthret = NULL;
		pthread_join(m_pthread, &pthret);
		return 1;
	}
#endif

	int Thread::start(int param_idx, void *param_ptr, JSTHREAD_THREADID_TYPE *pThreadId, const char *szThreadName)
	{
		int retval = 0;
		int nrst;
		int step = 0;

		JsCPPUtils::SmartPointer<Thread> spThread = this;

		m_index = param_idx;
		m_param = param_ptr;
		
		do
		{
#if defined(JSCUTILS_OS_LINUX)
			nrst = pthread_mutex_init(&m_run_mutex, NULL);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			step = 1;
			nrst = pthread_mutex_lock(&m_run_mutex);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}

			m_runningstatus = 1;

			step = 2;
			nrst = pthread_create(&m_pthread, NULL, threadProcV2, spThread.detach());
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			pThreadCtx->m_tid = m_pthread;

			if (szThreadName != NULL)
			{
				pthread_setname_np(m_pthread, szThreadName);
			}

			step = 3;
#elif defined(JSCUTILS_OS_WINDOWS)
			m_stop_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
			if (m_stop_hEvent == INVALID_HANDLE_VALUE)
			{
				nrst = GetLastError();
				retval = -nrst;
				break;
			}
			step = 1;
			m_runningstatus = 1;
			m_hThread = ::CreateThread(NULL, 0, threadProcV2, spThread.detach(), 0, &m_tid);
			if (m_hThread == INVALID_HANDLE_VALUE)
			{
				nrst = GetLastError();
				retval = -nrst;
				break;
			}
			step = 2;
#endif

			retval = 1;
		} while (0);

		if (retval <= 0)
		{
#if defined(JSCUTILS_OS_LINUX)
			if (step >= 2)
			{
				pthread_mutex_unlock(&m_run_mutex);
			}
			if (step >= 1)
			{
				pthread_mutex_destroy(&m_run_mutex);
			}
#elif defined(JSCUTILS_OS_WINDOWS)
			if (m_stop_hEvent != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_stop_hEvent);
				m_stop_hEvent = INVALID_HANDLE_VALUE;
			}
			if (m_hThread != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_hThread);
				m_hThread = INVALID_HANDLE_VALUE;
			}
#endif
		}
		else
		{
			if (pThreadId != NULL)
				*pThreadId = m_tid;
		}

		return retval;
	}
}
