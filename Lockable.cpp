/**
 * @file	Lockable.cpp
 * @class	Lockable
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/10/14
 * @brief	Lockable. It can help to thread-safe.
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "Lockable.h"

namespace JsCPPUtils
{
#if defined(JSCUTILS_OS_WINDOWS)
	Lockable::Lockable() : m_hMutex(NULL)
	{
		DWORD dwErr = 0;
		m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
		if (m_hMutex == INVALID_HANDLE_VALUE)
		{
			dwErr = GetLastError();
			throw -((int)dwErr);
		}
	}

	Lockable::~Lockable()
	{
		CloseHandle(m_hMutex);
		m_hMutex = INVALID_HANDLE_VALUE;
	}

	int Lockable::lock() const
	{
		DWORD dwWait;
		DWORD dwErr;
		dwWait = WaitForSingleObject(m_hMutex, INFINITE);
		if (dwWait != 0)
		{
			dwErr = GetLastError();
			return -((int)dwErr);
		}
		return 1;
	}

	int Lockable::unlock() const
	{
		DWORD dwErr;
		if (!ReleaseMutex(m_hMutex))
		{
			dwErr = GetLastError();
			return -((int)dwErr);
		}
		return 1;
	}
#elif defined(JSCUTILS_OS_LINUX)
	Lockable::Lockable()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}

	Lockable::~Lockable()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	int Lockable::lock() const
	{
		pthread_mutex_lock((pthread_mutex_t*)&m_mutex);
		return 1;
	}

	int Lockable::unlock() const
	{
		pthread_mutex_unlock((pthread_mutex_t*)&m_mutex);
		return 1;
	}
#endif
}
