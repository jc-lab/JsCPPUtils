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
#include <string.h>
#endif
#include <exception>

namespace JsCPPUtils
{
#if defined(JSCUTILS_OS_WINDOWS)
	Lockable::Lockable()
	{
		if (!::InitializeCriticalSectionAndSpinCount(&m_cs, 5000))
		{
			DWORD dwErr = ::GetLastError();
			throw std::exception("InitializeCriticalSectionAndSpinCount failed: ", dwErr);
		}
	}

	Lockable::~Lockable()
	{
		::DeleteCriticalSection(&m_cs);
	}

	int Lockable::lock() const
	{
		DWORD *pownertid = (DWORD*)&m_ownertid;
		::EnterCriticalSection((LPCRITICAL_SECTION)&m_cs);
		*pownertid =::GetCurrentThreadId();
		return 1;
	}

	int Lockable::trylock() const
	{
		DWORD *pownertid = (DWORD*)&m_ownertid;
		if (::TryEnterCriticalSection((LPCRITICAL_SECTION)&m_cs))
		{
			*pownertid =:: GetCurrentThreadId();
			return 1;
		}else{
			return 0;
		}
	}

	int Lockable::unlock() const
	{
		DWORD *pownertid = (DWORD*)&m_ownertid;
		*pownertid = 0;
		::LeaveCriticalSection((LPCRITICAL_SECTION)&m_cs);
		return 1;
	}

	LockableEx::LockableEx()
	{
		::InitializeCriticalSectionAndSpinCount(&m_cs, 5000);
	}

	LockableEx::~LockableEx()
	{
		::DeleteCriticalSection(&m_cs);
	}

	int LockableEx::lock() const
	{
		::EnterCriticalSection((LPCRITICAL_SECTION)&m_cs);
		return 1;
	}

	int LockableEx::unlock() const
	{
		::LeaveCriticalSection((LPCRITICAL_SECTION)&m_cs);
		return 1;
	}
	
	LockableRW::LockableRW()
	{
		HMODULE hModule = LoadLibrary(_T("KERNEL32.DLL"));
		m_syslock = 0;

		m_fnInitializeSRWLock = (fnInitializeSRWLock_t)GetProcAddress(hModule, "InitializeSRWLock");
		m_fnAcquireSRWLockExclusive = (fnInitializeSRWLock_t)GetProcAddress(hModule, "AcquireSRWLockExclusive");
		m_fnAcquireSRWLockShared = (fnInitializeSRWLock_t)GetProcAddress(hModule, "AcquireSRWLockShared");
		m_fnReleaseSRWLockExclusive = (fnInitializeSRWLock_t)GetProcAddress(hModule, "ReleaseSRWLockExclusive");
		m_fnReleaseSRWLockShared = (fnInitializeSRWLock_t)GetProcAddress(hModule, "ReleaseSRWLockShared");

		if (m_fnInitializeSRWLock)
		{
			m_fnInitializeSRWLock((PVOID*)&m_srwlock);
		}
	}
	
	LockableRW::~LockableRW()
	{
		
	}
	
	int LockableRW::writelock() const
	{
		if (m_fnInitializeSRWLock)
		{
			m_fnAcquireSRWLockExclusive((PVOID*)&m_srwlock);
			return 1;
		}

		while (true)
		{
			while (m_syslock & LF_WRITE_MASK)
				YieldProcessor();
			if ((InterlockedExchangeAdd((volatile LONG*)&m_syslock, LF_WRITE_FLAG) & LF_WRITE_MASK) == 0)
			{
				while (m_syslock & LF_READ_MASK)
					YieldProcessor();

				return 1;
			}
			InterlockedExchangeAdd((volatile LONG*)&m_syslock, -LF_WRITE_FLAG);
		}
	}
	
	int LockableRW::writeunlock() const
	{
		if (m_fnInitializeSRWLock)
		{
			m_fnReleaseSRWLockExclusive((PVOID*)&m_srwlock);
			return 1;
		}

		::InterlockedExchangeAdd((volatile LONG*)&m_syslock, -LF_WRITE_FLAG);
		return 1;
	}
	
	int LockableRW::readlock() const
	{
		if (m_fnInitializeSRWLock)
		{
			m_fnAcquireSRWLockShared((PVOID*)&m_srwlock);
			return 1;
		}

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
		if (m_fnInitializeSRWLock)
		{
			m_fnReleaseSRWLockShared((PVOID*)&m_srwlock);
			return 1;
		}

		::InterlockedDecrement((volatile LONG*)&m_syslock);
		return 1;
	}
	
#elif defined(JSCUTILS_OS_LINUX)
	Lockable::Lockable()
	{
		pthread_mutex_init(&m_mutex, NULL);
		m_ownertid = 0;
	}

	Lockable::~Lockable()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	int Lockable::lock() const
	{
		pthread_t *pownertid = (pthread_t*)&m_ownertid;
		pthread_mutex_lock((pthread_mutex_t*)&m_mutex);
		*pownertid = pthread_self();
		return 1;
	}

	int Lockable::trylock() const
	{
		int rc;
		pthread_t *pownertid = (pthread_t*)&m_ownertid;
		rc = pthread_mutex_trylock((pthread_mutex_t*)&m_mutex);
		if (rc == 0)
		{
			*pownertid = pthread_self();
			return 1;
		}else{
			return -rc;
		}
	}

	int Lockable::unlock() const
	{
		pthread_t *pownertid = (pthread_t*)&m_ownertid;
		*pownertid = 0;
		pthread_mutex_unlock((pthread_mutex_t*)&m_mutex);
		return 1;
	}
	
	LockableEx::LockableEx()
	{
		pthread_mutexattr_t a;
		pthread_mutexattr_init(&a);
		pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_mutex, &a);
		pthread_mutexattr_destroy(&a);
	}

	LockableEx::~LockableEx()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	int LockableEx::lock() const
	{
		pthread_mutex_lock((pthread_mutex_t*)&m_mutex);
		return 1;
	}

	int LockableEx::unlock() const
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
