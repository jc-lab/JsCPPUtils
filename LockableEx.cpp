/**
 * @file	LockableEx.cpp
 * @class	LockableEx
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/10/14
 * @brief	LockableEx. It can help to thread-safe. Lock count applied.
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "LockableEx.h"

namespace JsCPPUtils
{
	LockableEx::LockableEx()
		: m_lock(),
		m_tinfo(31, 16, 16, 4.0, 2.0, 32767)
	{
			
	}

	LockableEx::~LockableEx()
	{
			
	}

#if defined(JSCUTILS_OS_WINDOWS)
	int LockableEx::lock()
	{
		int nrst = 1;
		DWORD dwTid = GetCurrentThreadId();
		int &_locked = m_tinfo[dwTid];
		if (_locked++ == 0)
		{
			nrst = m_lock.lock();
		}
		return nrst;
	}

	int LockableEx::unlock(bool earseinmap)
	{
		int nrst = 1;
		DWORD dwTid = GetCurrentThreadId();
		int &_locked = m_tinfo[dwTid];
		if (--_locked == 0)
		{
			nrst = m_lock.unlock();
			if (earseinmap)
				m_tinfo.erase(dwTid);
		}
		return nrst;
	}
#elif defined(JSCUTILS_OS_LINUX)
	int LockableEx::lock()
	{
		int nrst;
		pthread_t curthread = pthread_self();
		int &_locked = m_tinfo[curthread];
		if (_locked++ == 0)
		{
			nrst = m_lock.lock();
		}
		return nrst;
	}

	int LockableEx::unlock(bool earseinmap)
	{
		int nrst;
		pthread_t curthread = pthread_self();
		int& _locked = m_tinfo[curthread];
		if (--_locked == 0)
		{
			nrst = m_lock.unlock();
			if (earseinmap)
				m_tinfo.erase(curthread);
		}
		return nrst;
	}
#endif
}
