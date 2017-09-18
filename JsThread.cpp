/**
 * @file	JsThread.cpp
 * @class	JsThread
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/11/03
 * @brief	JsThread
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <errno.h>

#include "JsThread.h"

namespace JsCPPUtils
{
	
#if defined(JSCUTILS_OS_LINUX)
	void *JsThread::threadProc(void *param)
	{
		JsCPPUtils::SmartPointer<ThreadContext> spThreadCtx;
		int retval;
		spThreadCtx.attach((JsCPPUtils::SmartPointer<ThreadContext>*)param);
		
		spThreadCtx->m_runningstatus = 2;
		retval = spThreadCtx->m_startroutine(spThreadCtx.getPtr(), spThreadCtx->m_index, spThreadCtx->m_param);
		spThreadCtx->m_runningstatus = 3;
		
		return (void*)retval;
	}
#elif defined(JSCUTILS_OS_WINDOWS)
	DWORD WINAPI JsThread::threadProc(LPVOID param)
	{
		JsCPPUtils::SmartPointer<ThreadContext> spThreadCtx;
		int retval;
		spThreadCtx.attach((JsCPPUtils::SmartPointer<ThreadContext>*)param);
		
		spThreadCtx->m_runningstatus = 2;
		retval = spThreadCtx->m_startroutine(spThreadCtx.getPtr(), spThreadCtx->m_index, spThreadCtx->m_param);
		spThreadCtx->m_runningstatus = 3;
		
		return (DWORD)retval;
	}
#endif

	int JsThread::start(JsCPPUtils::SmartPointer<ThreadContext> *pspThreadCtx, StartRoutine_t startroutine, int param_idx, void *param_ptr, JSTHREAD_THREADID_TYPE *pThreadId, const char *szThreadName)
	{
		int retval = 0;
		int nrst;
		int step = 0;
	
		ThreadContext *pThreadCtx;
		JsCPPUtils::SmartPointer< ThreadContext > spThreadCtx;
		
		pThreadCtx = new ThreadContext(startroutine, param_idx, param_ptr);
		if(pThreadCtx == NULL)
		{
			return -errno;
		}

		spThreadCtx = pThreadCtx;

		do
		{
#if defined(JSCUTILS_OS_LINUX)
			nrst = pthread_mutex_init(&pThreadCtx->m_run_mutex, NULL);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			step = 1;
			nrst = pthread_mutex_lock(&pThreadCtx->m_run_mutex);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			
			pThreadCtx->m_runningstatus = 1;
			
			step = 2;
			nrst = pthread_create(&pThreadCtx->m_pthread, NULL, threadProc, spThreadCtx.detach());
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			pThreadCtx->m_tid = pThreadCtx->m_pthread;
			
			if (szThreadName != NULL)
			{
				pthread_setname_np(pThreadCtx->m_pthread, szThreadName);
			}
			
			step = 3;
#elif defined(JSCUTILS_OS_WINDOWS)
			pThreadCtx->m_stop_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
			if(pThreadCtx->m_stop_hEvent == INVALID_HANDLE_VALUE)
			{
				nrst = GetLastError();
				retval = -nrst;
				break;
			}
			step = 1;
			pThreadCtx->m_runningstatus = 1;
			pThreadCtx->m_hThread = ::CreateThread(NULL, 0, threadProc, spThreadCtx.detach(), 0, &pThreadCtx->m_tid);
			if(pThreadCtx->m_hThread == INVALID_HANDLE_VALUE)
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
				pthread_mutex_unlock(&pThreadCtx->m_run_mutex);
			}
			if (step >= 1)
			{
				pthread_mutex_destroy(&pThreadCtx->m_run_mutex);
			}
#elif defined(JSCUTILS_OS_WINDOWS)
			if(pThreadCtx->m_stop_hEvent != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(pThreadCtx->m_stop_hEvent);
				pThreadCtx->m_stop_hEvent = INVALID_HANDLE_VALUE;
			}
			if(pThreadCtx->m_hThread != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(pThreadCtx->m_hThread);
				pThreadCtx->m_hThread = INVALID_HANDLE_VALUE;
			}
#endif
		}
		else
		{
			if (pspThreadCtx != NULL)
				*pspThreadCtx = spThreadCtx;
			if(pThreadId != NULL)
				*pThreadId = pThreadCtx->m_tid;
		}
	
		return retval;
	}

	int JsThread::reqStop(ThreadContext *pThreadCtx)
	{
		return pThreadCtx->reqStop();
	}

	int JsThread::ThreadContext::reqStop()
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

	int JsThread::ThreadContext::_inthread_isRun()
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
	
	JsThread::RunningStatus JsThread::ThreadContext::getRunningStatus()
	{
		return (RunningStatus)m_runningstatus.get();
	}

#if defined(JSCUTILS_OS_WINDOWS)
	DWORD JsThread::ThreadContext::getThreadId()
	{
		return m_tid;
	}
#elif defined(JSCUTILS_OS_LINUX)
	pthread_t JsThread::ThreadContext::getThreadId()
	{
		return m_tid;
	}
#endif
	
	
}
