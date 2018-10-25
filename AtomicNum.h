/**
 * @file	AtomicNum.h
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
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
#include <new>

#ifdef new
#pragma push_macro("new")
#undef new
#define SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_NEW 1
#endif

#ifdef delete
#pragma push_macro("delete")
#undef delete
#define SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_DELETE 1
#endif

namespace JsCPPUtils
{
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
		virtual T incget() = 0;
		virtual T decget() = 0;
		virtual void operator+=(T y) = 0;
		virtual void operator-=(T y) = 0;
		virtual void operator++() = 0;
		virtual void operator--() = 0;
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
			return (T)m_value; // ::InterlockedExchangeAdd64((volatile LONGLONG *)&m_value, 0);
		}

		T get() const
		{
			return (T)m_value; // ::InterlockedExchangeAdd64((volatile LONGLONG *)&m_value, 0);
		}

		T getset(T value)
		{
			return (T)::InterlockedExchange64(&m_value, (LONGLONG)value);
		}

		T getifset(T value, T ifvalue)
		{
			return (T)::InterlockedCompareExchange64(&m_value, (LONGLONG)value, (LONGLONG)ifvalue);
		}

		T getadd(T y)
		{
			return (T)::InterlockedExchangeAdd64(&m_value, (LONGLONG)y);
		}

		T incget()
		{
			return (T)::InterlockedIncrement64(&m_value);
		}

		T decget()
		{
			return (T)::InterlockedDecrement64(&m_value);
		}

		void operator+=(T y)
		{
			::InterlockedExchangeAdd64(&m_value, (LONGLONG)y);
		}

		void operator-=(T y)
		{
			::InterlockedExchangeAdd64(&m_value, -((LONGLONG)y));
		}

		void operator++()
		{
			::InterlockedIncrement64(&m_value);
		}

		void operator--()
		{
			::InterlockedDecrement64(&m_value);
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
			return (((T)m_value) == y);
		}

		bool operator!=(T y) const
		{
			return (((T)m_value) != y);
		}

		bool operator>(T y) const
		{
			T tval = ((T)m_value);
			return (tval > y);
		}

		bool operator<(T y) const
		{
			T tval = ((T)m_value);
			return (tval < y);
		}

		bool operator>=(T y) const
		{
			T tval = ((T)m_value);
			return (tval >= y);
		}

		bool operator<=(T y) const
		{
			T tval = ((T)m_value);
			return (tval <= y);
		}
	};
#endif
//#else
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
			return (T)m_value; // ::InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
		}

		T get() const
		{
			return (T)m_value; // ::InterlockedExchangeAdd((volatile LONG *)&m_value, 0);
		}

		T getset(T value)
		{
			return (T)::InterlockedExchange(&m_value, (LONG)value);
		}

		T getifset(T value, T ifvalue)
		{
			return (T)::InterlockedCompareExchange(&m_value, (LONG)value, (LONG)ifvalue);
		}

		T incget()
		{
			return (T)::InterlockedIncrement(&m_value);
		}

		T decget()
		{
			return (T)::InterlockedDecrement(&m_value);
		}

		void operator+=(T y)
		{
			::InterlockedExchangeAdd(&m_value, (LONG)y);
		}

		void operator-=(T y)
		{
			::InterlockedExchangeAdd(&m_value, -((LONG)y));
		}

		void operator++()
		{
			::InterlockedIncrement(&m_value);
		}

		void operator--()
		{
			::InterlockedDecrement(&m_value);
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
			T tval = ((T)m_value);
			return (tval == y);
		}

		bool operator!=(T y) const
		{
			T tval = ((T)m_value);
			return (tval != y);
		}

		bool operator>(T y) const
		{
			T tval = ((T)m_value);
			return (tval > y);
		}

		bool operator<(T y) const
		{
			T tval = ((T)m_value);
			return (tval < y);
		}

		bool operator>=(T y) const
		{
			T tval = ((T)m_value);
			return (tval >= y);
		}

		bool operator<=(T y) const
		{
			T tval = ((T)m_value);
			return (tval <= y);
		}
	};

//#endif

	template <typename T>
	class basic_AtomicNumMutex : public basic_AtomicNumAbstract<T>, private JsCPPUtils::Lockable
	{
	private:
		volatile T m_value;

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

		T incget()
		{
			T value;
			lock();
			value = ++m_value;
			unlock();
			return value;
		}

		T decget()
		{
			T value;
			lock();
			value = --m_value;
			unlock();
			return value;
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

		void operator++()
		{
			lock();
			m_value++;
			unlock();
		}

		void operator--()
		{
			lock();
			m_value--;
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

	template <typename T>
	class AtomicNum
	{
	private:
		basic_AtomicNumAbstract<T> *m_pimpl;

#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
		JsCUtils_fnMalloc_t m_custom_malloc;
		JsCUtils_fnRealloc_t m_custom_realloc;
		JsCUtils_fnFree_t m_custom_free;
#endif

		void _init(T initialvalue)
		{
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
#if defined(WIN64) || defined(_WIN64)
			if(sizeof(T) <= 8)
			{
				if (m_custom_malloc)
				{
					m_pimpl = (basic_AtomicNumSYS64<T>*)m_custom_malloc(sizeof(basic_AtomicNumSYS64<T>));
					new(m_pimpl) basic_AtomicNumSYS64<T>(initialvalue);
				}
				else 
					m_pimpl = new basic_AtomicNumSYS64<T>(initialvalue);
			}else
#else
			if(sizeof(T) <= 4)
			{
				if (m_custom_malloc)
				{
					m_pimpl = (basic_AtomicNumSYS32<T>*)m_custom_malloc(sizeof(basic_AtomicNumSYS32<T>));
					new(m_pimpl) basic_AtomicNumSYS32<T>(initialvalue);
				}
				else
					m_pimpl = new basic_AtomicNumSYS32<T>(initialvalue);
			}else
#endif
			{
				if (m_custom_malloc)
				{
					m_pimpl = (basic_AtomicNumMutex<T>*)m_custom_malloc(sizeof(basic_AtomicNumMutex<T>));
					new(m_pimpl) basic_AtomicNumMutex<T>(initialvalue);
				}
				else
					m_pimpl = new basic_AtomicNumMutex<T>(initialvalue);
			}
#else
#if defined(WIN64) || defined(_WIN64)
			if(sizeof(T) <= 8)
			{
				m_pimpl = new basic_AtomicNumSYS64<T>(initialvalue);
			}else
#else
			if(sizeof(T) <= 4)
			{
				m_pimpl = new basic_AtomicNumSYS32<T>(initialvalue);
			}else
#endif
			{
				m_pimpl = new basic_AtomicNumMutex<T>(initialvalue);
			}
#endif
		}

	public:
		AtomicNum()
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			:
			m_custom_malloc(NULL),
			m_custom_realloc(NULL),
			m_custom_free(NULL)
#endif
		{
			_init(0);
		}

		AtomicNum(int initialvalue
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			, JsCUtils_fnMalloc_t _custom_malloc = NULL
			, JsCUtils_fnRealloc_t _custom_realloc = NULL
			, JsCUtils_fnFree_t _custom_free = NULL
#endif
		)
		{
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			m_custom_malloc = _custom_malloc;
			m_custom_realloc = _custom_realloc;
			m_custom_free = _custom_free;
#endif
			_init(initialvalue);
		}
	
		~AtomicNum()
		{
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			if(m_pimpl != NULL)
			{
				if (m_custom_free)
				{
					m_pimpl->~basic_AtomicNumAbstract<T>();
					m_custom_free(m_pimpl);
					m_pimpl = NULL;
				} else {
					delete m_pimpl;
					m_pimpl = NULL;
				}
			}
#else
			if (m_pimpl != NULL)
			{
				delete m_pimpl;
				m_pimpl = NULL;
			}
#endif
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
			T resvalue;
			resvalue = m_pimpl->getset(value);
			return resvalue;
		}

		T getifset(T value, T ifvalue)
		{
			T resvalue;
			resvalue = m_pimpl->getifset(value, ifvalue);
			return resvalue;
		}

		T incget()
		{
			T resvalue;
			resvalue = m_pimpl->incget();
			return resvalue;
		}

		T decget()
		{
			T resvalue;
			resvalue = m_pimpl->decget();
			return resvalue;
		}

		void operator+=(T y)
		{
			m_pimpl->operator+=(y);
		}

		void operator-=(T y)
		{
			m_pimpl->operator-=(y);
		}

		void operator++()
		{
			m_pimpl->operator++(y);
		}

		void operator--()
		{
			m_pimpl->operator--(y);
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
}

#ifdef SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_NEW
#undef SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_NEW
#pragma pop_macro("new")
#endif

#ifdef SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_DELETE
#undef SRC_77D1AD00DE4111E78F1A0800200C9A66_DEFINED_OLD_DELETE
#pragma pop_macro("delete")
#endif


#endif
