/**
 * @file	AtomicNum.h
 * @class	AtomicNum
 * @brief	Thread-Safe한 숫자형을 구현한 클래스
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_ATOMICNUM_H__
#define __JSCPPUTILS_ATOMICNUM_H__

#include "Lockable.h"
#include <windows.h>
#include <intrin.h>
#include <typeinfo>

namespace JsCPPUtils
{
#if 1

	template <typename T>
	class basic_AtomicNumAbstract
	{
	public:
		virtual void set(T value) = 0;
		virtual void operator=(T value) = 0;
		virtual operator T() const = 0;
		virtual T get() const = 0;
		virtual T getset(T value) = 0;
		virtual T getifset(T value, T ifvalue) = 0;
		virtual void operator+=(T y) = 0;
		virtual void operator-=(T y) = 0;
		virtual void operator&=(T y) = 0;
		virtual void operator|=(T y) = 0;
		virtual bool operator==(T y) const = 0;
		virtual bool operator!=(T y) const = 0;
		virtual bool operator>(T y) const = 0;
		virtual bool operator<(T y) const = 0;
		virtual bool operator>=(T y) const = 0;
		virtual bool operator<=(T y) const = 0;
	};

#if defined(WIN64) || defined(_WIN64)
	template <typename T = LONGLONG>
	class basic_AtomicNumSYS64 : public basic_AtomicNumAbstract<T>
	{
	private:
		volatile LONGLONG m_value;

	public:
		basic_AtomicNumSYS64() : 
			m_value(0)
		{
		}

		basic_AtomicNumSYS64(LONGLONG initialvalue) : 
			m_value(initialvalue)
		{
		}
	
		~basic_AtomicNumSYS64()
		{
		}

		void set(T value)
		{
			::InterlockedExchange64(&m_value, value);
		}
		
		void operator=(T value)
		{
			::InterlockedExchange64(&m_value, value);
		}

		operator T() const
		{
			return (T)::InterlockedExchangeAdd64((volatile LONGLONG *)&m_value, 0);
		}

		T get() const
		{
			return (T)::InterlockedExchangeAdd64((volatile LONGLONG *)&m_value, 0);
		}

		T getset(T value)
		{
			return (T)::InterlockedExchange64(&m_value, (LONGLONG)value);
		}

		T getifset(T value, T ifvalue)
		{
			return (T)::InterlockedCompareExchange64(&m_value, (LONGLONG)value, (LONGLONG)ifvalue);
		}

		void operator+=(T y)
		{
			::InterlockedExchangeAdd64(&m_value, (LONGLONG)y);
		}

		void operator-=(T y)
		{
			::InterlockedExchangeAdd64(&m_value, -((LONGLONG)y));
		}

		void operator&=(T y)
		{
			::InterlockedAnd64(&m_value, (LONGLONG)y);
		}

		void operator|=(T y)
		{
			::InterlockedOr64(&m_value, (LONGLONG)y);
		}

		bool operator==(T y) const
		{
			return (((T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0)) == y);
		}

		bool operator!=(T y) const
		{
			return (((T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0)) != y);
		}

		bool operator>(T y) const
		{
			T tval = (T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0);
			return (tval > y);
		}

		bool operator<(T y) const
		{
			T tval = (T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0);
			return (tval < y);
		}

		bool operator>=(T y) const
		{
			T tval = (T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0);
			return (tval >= y);
		}

		bool operator<=(T y) const
		{
			T tval = (T)InterlockedExchange64((volatile LONGLONG *)&m_value, 0);
			return (tval <= y);
		}
	};

#else
	template <typename T = LONG>
	class basic_AtomicNumSYS32 : public basic_AtomicNumAbstract<T>
	{
	private:
		volatile LONG m_value;

	public:
		basic_AtomicNumSYS32() : 
			m_value(0)
		{
		}

		basic_AtomicNumSYS32(LONG initialvalue) : 
			m_value(initialvalue)
		{
		}
	
		~basic_AtomicNumSYS32()
		{
		}

		void set(T value)
		{
			::InterlockedExchange(&m_value, (LONG)value);
		}
		
		void operator=(T value)
		{
			::InterlockedExchange(&m_value, (LONG)value);
		}

		operator T() const
		{
			return (T)::InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
		}

		T get() const
		{
			return (T)::InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
		}

		T getset(T value)
		{
			return (T)::InterlockedExchange(&m_value, (LONG)value);
		}

		T getifset(T value, T ifvalue)
		{
			return (T)::InterlockedCompareExchange(&m_value, (LONG)value, (LONG)ifvalue);
		}

		void operator+=(T y)
		{
			::InterlockedExchangeAdd(&m_value, ((LONG)y));
		}

		void operator-=(T y)
		{
			::InterlockedExchangeAdd(&m_value, -((LONG)y));
		}

		void operator&=(T y)
		{
			::_InterlockedAnd(&m_value, ((LONG)y));
		}

		void operator|=(T y)
		{
			::_InterlockedOr(&m_value, ((LONG)y));
		}

		bool operator==(T y) const
		{
			return (((T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0)) == (y));
		}

		bool operator!=(T y) const
		{
			return (((T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0)) != (y));
		}

		bool operator>(T y) const
		{
			T tval = (T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
			return (tval > y);
		}

		bool operator<(T y) const
		{
			T tval = (T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
			return (tval < y);
		}

		bool operator>=(T y) const
		{
			T tval = (T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
			return (tval >= y);
		}

		bool operator<=(T y) const
		{
			T tval = (T)InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
			return (tval <= y);
		}
	};

	template <typename T>
	class basic_AtomicNumMutex : public basic_AtomicNumAbstract<T>, private JsCPPUtils::Lockable
	{
	private:
		T m_value;

	public:
		basic_AtomicNumMutex() : 
			m_value(0)
		{
		}

		basic_AtomicNumMutex(T initialvalue) : 
			m_value(initialvalue)
		{
		}
	
		~basic_AtomicNumMutex()
		{
		}

		void set(T value)
		{
			lock();
			m_value = value;
			unlock();
		}
		
		void operator=(T value)
		{
			lock();
			m_value = value;
			unlock();
		}

		operator T() const
		{
			T value;
			lock();
			value = m_value;
			unlock();
			return value;
		}

		T get() const
		{
			T value;
			lock();
			value = m_value;
			unlock();
			return value;
		}

		T getset(T value)
		{
			T old;
			lock();
			old = m_value;
			m_value = value;
			unlock();
			return old;
		}

		T getifset(T value, T ifvalue)
		{
			T old;
			lock();
			old = m_value;
			if(old == ifvalue)
				m_value = value;
			unlock();
			return old;
		}

		void operator+=(T y)
		{
			lock();
			m_value += y;
			unlock();
		}

		void operator-=(T y)
		{
			lock();
			m_value -= y;
			unlock();
		}

		void operator&=(T y)
		{
			lock();
			m_value &= y;
			unlock();
		}

		void operator|=(T y)
		{
			lock();
			m_value |= y;
			unlock();
		}

		bool operator==(T y) const
		{
			bool r;
			lock();
			r = m_value == y;
			unlock();
			return r;
		}

		bool operator!=(T y) const
		{
			bool r;
			lock();
			r = m_value != y;
			unlock();
			return r;
		}

		bool operator>(T y) const
		{
			bool r;
			lock();
			r = m_value > y;
			unlock();
			return r;
		}

		bool operator<(T y) const
		{
			bool r;
			lock();
			r = m_value < y;
			unlock();
			return r;
		}

		bool operator>=(T y) const
		{
			bool r;
			lock();
			r = m_value >= y;
			unlock();
			return r;
		}

		bool operator<=(T y) const
		{
			bool r;
			lock();
			r = m_value <= y;
			unlock();
			return r;
		}
	};
#endif

	template <typename T>
	class AtomicNum
	{
	private:
		basic_AtomicNumAbstract<T> *m_pimpl;

		void _init(T initialvalue)
		{
#if defined(WIN64) || defined(_WIN64)
			if(sizeof(T) <= 8)
			{
				m_pimpl = new basic_AtomicNumSYS64<T>(initialvalue);
			}else
#endif
			if(sizeof(T) <= 4)
			{
				m_pimpl = new basic_AtomicNumSYS32<T>(initialvalue);
			}else{
				m_pimpl = new basic_AtomicNumMutex<T>(initialvalue);
			}
		}

	public:
		AtomicNum()
		{
			_init(0);
		}

		AtomicNum(int initialvalue)
		{
			_init(initialvalue);
		}
	
		~AtomicNum()
		{
			if(m_pimpl != NULL)
			{
				delete m_pimpl;
				m_pimpl = NULL;
			}
		}

		void set(T value)
		{
			m_pimpl->set(value);
		}
		
		void operator=(T value)
		{
			m_pimpl->set(value);
		}

		operator T() const
		{
			return m_pimpl->get();
		}

		T get() const
		{
			return m_pimpl->get();
		}

		T getset(T value)
		{
			return m_pimpl->getset(value);
		}

		T getifset(T value, T ifvalue)
		{
			return m_pimpl->getifset(value, ifvalue);
		}

		void operator+=(T y)
		{
			m_pimpl->operator+=(y);
		}

		void operator-=(T y)
		{
			m_pimpl->operator-=(y);
		}

		void operator&=(T y)
		{
			m_pimpl->operator&=(y);
		}

		void operator|=(T y)
		{
			m_pimpl->operator|=(y);
		}

		bool operator==(T y) const
		{
			return m_pimpl->operator==(y);
		}

		bool operator!=(T y) const
		{
			return m_pimpl->operator!=(y);
		}

		bool operator>(T y)
		{
			return m_pimpl->operator>(y);
		}

		bool operator<(T y) const
		{
			return m_pimpl->operator<(y);
		}

		bool operator>=(T y) const
		{
			return m_pimpl->operator>=(y);
		}

		bool operator<=(T y) const
		{
			return m_pimpl->operator<=(y);
		}
	};


#else
	template <typename T, bool _usebymanythread = false>
	class AtomicNum : public LockableEx
	{
	private:
		volatile T m_value;

	public:
		AtomicNum() : 
			LockableEx(),
			m_value(0)
		{
		}

		AtomicNum(int initialvalue) : 
			LockableEx(),
			m_value(initialvalue)
		{
		}
	
		~AtomicNum()
		{
		}

		void set(T y)
		{
			LockableEx::lock();
			m_value = y;
			LockableEx::unlock(_usebymanythread);
		}
		
		void operator=(T y)
		{
			LockableEx::lock();
			m_value = y;
			LockableEx::unlock(_usebymanythread);
		}

		operator T()
		{
			T value;
			LockableEx::lock();
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		T get()
		{
			T value;
			LockableEx::lock();
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		T getset(T value)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}

		T getifset(T value, T ifvalue)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			if(old == ifvalue)
				m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}

		T getifnset(T value, T ifnvalue)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			if(old != ifnvalue)
				m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}
		
		/*
#if defined(_MSC_VER)
		AtomicNum& operator--()
		{
			T value;
			LockableEx::lock();
			m_value -= 1;
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return *this;
		}

		T operator--(int)
		{
			T value;
			LockableEx::lock();
			value = m_value;
			m_value -= 1;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		AtomicNum& operator++()
		{
			T value;
			LockableEx::lock();
			m_value += 1;
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return *this;
		}

		T operator++(int)
		{
			T value;
			LockableEx::lock();
			value = m_value;
			m_value += 1;
			LockableEx::unlock(_usebymanythread);
			return value;
		}
#endif
		*/
		
		void operator+=(T y)
		{
			LockableEx::lock();
			m_value += y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator-=(T y)
		{
			LockableEx::lock();
			m_value -= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator*=(T y)
		{
			LockableEx::lock();
			m_value *= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator/=(T y)
		{
			LockableEx::lock();
			m_value /= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator%=(T y)
		{
			LockableEx::lock();
			m_value %= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator<<=(T y)
		{
			LockableEx::lock();
			m_value <<= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator>>=(T y)
		{
			LockableEx::lock();
			m_value >>= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator^=(T y)
		{
			LockableEx::lock();
			m_value ^= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator&=(T y)
		{
			LockableEx::lock();
			m_value &= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator|=(T y)
		{
			LockableEx::lock();
			m_value |= y;
			LockableEx::unlock(_usebymanythread);
		}

		bool operator==(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value == y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator!=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value != y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator>(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value > y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator<(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value < y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator>=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value >= y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator<=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value <= y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}
	};
#endif

}

#endif
