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
#elif defined(JSCUTILS_OS_WINDOWS)
		m_hThread = INVALID_HANDLE_VALUE;;
#endif
		m_retval = 0;
	}
	
	Thread::~Thread()
	{
		if (m_runningstatus.get() > 0)
		{
			reqStop();
			join();
		}
#if defined(JSCUTILS_OS_LINUX)
		::pthread_mutex_destroy(&m_run_mutex);
#elif defined(JSCUTILS_OS_WINDOWS)
		if (m_hThread && (m_hThread != INVALID_HANDLE_VALUE))
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

		retval = spThread->run(spThread->m_index, spThread->m_param);
		spThread->m_runningstatus.set(0);

		return (DWORD)retval;
	}
#elif defined(JSCUTILS_OS_WINDOWS)
	DWORD WINAPI Thread::threadProcV2(LPVOID param)
	{
		JsCPPUtils::SmartPointer<Thread> spThread;
		int retval;
		spThread.attach((JsCPPUtils::SmartPointer<Thread>*)param);

		spThread->m_retval = retval = spThread->run(spThread->m_index, spThread->m_param);
		spThread->m_runningstatus.set(0);

		return (DWORD)retval;
	}
#endif

	int Thread::reqStop()
	{
		return m_runningstatus.getifset(2, 1);
	}

	int Thread::isRunning()
	{
		return m_runningstatus.get();
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
		while (m_runningstatus.get() > 0)
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

		if (!JsCPPUtils::SmartPointer<Thread>::checkManaged(this))
		{
			assert(JsCPPUtils::SmartPointer<Thread>::checkManaged(this));
			return -1;
		}

		JsCPPUtils::SmartPointer<Thread> spThread = this;

		m_index = param_idx;
		m_param = param_ptr;
		
		do
		{
#if defined(JSCUTILS_OS_LINUX)
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
#if defined(JSCUTILS_OS_WINDOWS)
			if (m_hThread && (m_hThread != INVALID_HANDLE_VALUE))
			{
				::CloseHandle(m_hThread);
				m_hThread = NULL;
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
