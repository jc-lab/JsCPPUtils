/**
 * @file	WeakSmartPointer.h
 * @class	WeakSmartPointer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/11/14
 * @brief	WeakSmartPointer. Can be check object is alive.
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_WEAKSMARTPOINTER_H__
#define __JSCPPUTILS_WEAKSMARTPOINTER_H__

#include <stdio.h>
#include <assert.h>
#include <exception>

#include "SmartPointer.h"

namespace JsCPPUtils
{
	template <class T>
		class WeakSmartPointer : public _JsCPPUtils_private::SmartPointerBase
		{
		private:
			T* ptr;

		public:
			explicit WeakSmartPointer()
			{
				_constructor();
			}

			WeakSmartPointer(int maybeNull)
			{
				_constructor();
			}

			WeakSmartPointer(const WeakSmartPointer& _ref)
			{
				copyFrom(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			template<class U>
			WeakSmartPointer(const WeakSmartPointer<U>& _ref)
			{
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			template<class U>
			void operator=(const WeakSmartPointer<U>& _ref)
			{
				delWeakRef();
				_constructor();
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			void operator=(const WeakSmartPointer<T>& _ref)
			{
				delWeakRef();
				_constructor();
				copyFrom(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			template<class U>
			WeakSmartPointer(const SmartPointer<U>& _ref)
			{
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			template<class U>
			void operator=(const SmartPointer<U>& _ref)
			{
				delWeakRef();
				_constructor();
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

			void operator=(const SmartPointer<T>& _ref)
			{
				delWeakRef();
				_constructor();
				copyFrom(_ref);
				this->ptr = (T*)m_ptr;
				addWeakRef();
			}

#if (__cplusplus >= 201103) || (defined(HAS_MOVE_SEMANTICS) && HAS_MOVE_SEMANTICS == 1)
			explicit WeakSmartPointer(WeakSmartPointer&& _ref)
			{
				m_ptr = _ref->m_ptr;
				this->ptr = (T*)m_ptr;
				m_rootManager = _ref->m_rootManager;
				_ref->m_ptr = NULL;
				_ref->m_rootManager = NULL;
			}
#endif
				
			~WeakSmartPointer()
			{
				delWeakRef();
			}

			T* operator->() const { return (T*)m_ptr; }
			T& operator*() const { return *(T*)m_ptr; }
			T* getPtr() const { return (T*)m_ptr; }

			bool operator==(void *x) const { return m_ptr == x; }
			bool operator!=(void *x) const { return m_ptr != x; }
			bool operator!() const { return (m_ptr == NULL); }

			template<class U>
			bool operator==(const SmartPointer<U>& x) const { return m_ptr == x.m_ptr; }
			template<class U>
			bool operator!=(const SmartPointer<U>& x) const { return m_ptr != x.m_ptr; }
			template<class U>
			bool operator==(const WeakSmartPointer<U>& x) const { return m_ptr == x.m_ptr; }
			template<class U>
			bool operator!=(const WeakSmartPointer<U>& x) const { return m_ptr != x.m_ptr; }

			bool isAlive()
			{
				if (!m_refcounter)
					return false;
				return m_refcounter->alive.get() == 1;
			}

			SmartPointer<T> getSmartPointer()
			{
				SmartPointer<T> object;
				void *dptr = detach();
				if (dptr == NULL)
					return object;
				object.attach(dptr);
				return object;
			}

			void reset()
			{
				this->ptr = NULL;
				delWeakRef();
				_constructor();
			}
		};
}

#endif /* __JSCPPUTILS_SMARTPOINTER_H__ */
