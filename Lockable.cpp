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
#if defined(JSCUTILS_OS_LINUX)
#include <errno.h>
#include <unistd.h>
#endif

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
	
	
	LockableRW::LockableRW()
	{
		m_syslock = 0;
	}
	
	LockableRW::~LockableRW()
	{
		
	}
	
	int LockableRW::writelock() const
	{
		while (1)
		{
			while (m_syslock & LF_WRITE_MASK)
				::YieldProcessor();
			if ((::InterlockedExchangeAdd((volatile LONG*)&m_syslock, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)
			{
				while (m_syslock & LF_READ_MASK)
					::YieldProcessor();
				return 1;
			}
			::InterlockedExchangeAdd((volatile LONG*)&m_syslock, -LF_WRITE_FLAG);
		}
		return 1;
	}
	
	int LockableRW::writeunlock() const
	{
		::InterlockedExchangeAdd((volatile LONG*)&m_syslock, -LF_WRITE_FLAG);
		return 1;
	}
	
	int LockableRW::readlock() const
	{
		while (1)
		{
			while (m_syslock & LF_WRITE_MASK)
				YieldProcessor();
			
			if ((InterlockedIncrement((volatile LONG*)&m_syslock) & LF_WRITE_MASK) == 0)
				return 1;
			else
				InterlockedDecrement((volatile LONG*)&m_syslock);
		}
		return 1;
	}
	
	int LockableRW::readunlock() const
	{
		::InterlockedDecrement((volatile LONG*)&m_syslock);
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
	
	
	LockableRW::LockableRW()
	{
		pthread_rwlock_init(&m_syslock, NULL);
	}
	
	LockableRW::~LockableRW()
	{
		pthread_rwlock_destroy(&m_syslock);
	}
	
	int LockableRW::writelock() const
	{
		pthread_rwlock_wrlock((pthread_rwlock_t*)&m_syslock);
		return 1;
	}
	
	int LockableRW::writeunlock() const
	{
		pthread_rwlock_unlock((pthread_rwlock_t*)&m_syslock);
		return 1;
	}
	
	int LockableRW::readlock() const
	{
		int rc;
		while ((rc = pthread_rwlock_rdlock((pthread_rwlock_t*)&m_syslock)) == EAGAIN)
		{
			::usleep(1);
		}
		return 1;
	}
	
	int LockableRW::readunlock() const
	{
		pthread_rwlock_unlock((pthread_rwlock_t*)&m_syslock);
		return 1;
	}
#endif
}
