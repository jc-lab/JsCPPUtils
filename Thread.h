/**
 * @file	thread.h
 * @class	thread
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/10/08
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_THREAD_H__
#define __JSCPPUTILS_THREAD_H__

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
	class Thread : public SmartPointerRefCounter
	{
	public:
		enum RunningStatus
		{
			RS_INITALIZING = 0,
			RS_INITALIZED = 1,
			RS_STARTED = 2,
			RS_STOPPED = 3
		};

	private:
		int m_index;
		void *m_param;

		JSTHREAD_THREADID_TYPE m_tid;

#if defined(JSCUTILS_OS_LINUX)
		pthread_t m_pthread;
		pthread_mutex_t m_run_mutex;
#elif defined(JSCUTILS_OS_WINDOWS)
		HANDLE m_hThread;
		HANDLE m_stop_hEvent;
#endif

		int m_retval;
		JsCPPUtils::AtomicNum<int> m_runningstatus;

#if defined(JSCUTILS_OS_LINUX)
		static void *threadProcV2(void *param);
#elif defined(JSCUTILS_OS_WINDOWS)
		static DWORD WINAPI threadProcV2(LPVOID param);
#endif

	public:
		Thread();
		virtual ~Thread();

		int start(int param_idx = 0, void *param_ptr = NULL, JSTHREAD_THREADID_TYPE *pThreadId = NULL, const char *szThreadName = NULL);
		int reqStop();
		int isRunning();
		RunningStatus getRunningStatus();
#if defined(JSCUTILS_OS_LINUX)
		int join(int nTimeout = -1);
		pthread_t getThreadId();
#elif defined(JSCUTILS_OS_WINDOWS)
		int join(DWORD dwTimeout = INFINITE);
		DWORD getThreadId();
#endif

	protected:
		bool isRun() {
			return m_runningstatus.get() == 2;
		}
		virtual int run(int param_idx, void *param_ptr) = 0;
	};
};

#endif /* __JSCPPUTILS_JSTHREAD_H__ */
