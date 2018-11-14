/**
 * @file	SmartPointer.h
 * @class	SmartPointer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/09/30
 * @brief	SmartPointer (thread-safe). It can be used as a smart object by inheritance.
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_SMARTPOINTER_H__
#define __JSCPPUTILS_SMARTPOINTER_H__

#include <stdio.h>
#include <assert.h>
#include <exception>

#include "AtomicNum.h"

 /******************** Loki begin ********************/

#ifndef __JSCPPUTILS_PRIVATE_LOKI_SUPERCLASSCHECKER
#define __JSCPPUTILS_PRIVATE_LOKI_SUPERCLASSCHECKER

namespace _JsCPPUtils_private
{
	namespace Loki
	{
		namespace Private
		{
			template <class T, class U>
			struct ConversionHelper
			{
				typedef char Small;
				struct Big { char dummy[2]; };
				static Big   Test(...);
				static Small Test(U);
				static T MakeT();
			};
		}
		template <class T, class U>
		struct Conversion
		{
			typedef Private::ConversionHelper<T, U> H;
			enum { exists = sizeof(typename H::Small) == sizeof((H::Test(H::MakeT()))) };
			enum { exists2Way = exists && Conversion < U, T > ::exists };
			enum { sameType = false };
		};

		template <class T>
		struct Conversion<T, T>
		{
			enum { exists = 1, exists2Way = 1, sameType = 1 };
		};

		template <class T>
		struct Conversion<void, T>
		{
			enum { exists = 0, exists2Way = 0, sameType = 0 };
		};

		template <class T>
		struct Conversion<T, void>
		{
			enum { exists = 0, exists2Way = 0, sameType = 0 };
		};

		template <>
		struct Conversion<void, void>
		{
		public:
			enum { exists = 1, exists2Way = 1, sameType = 1 };
		};


		template <class T, class U>
		struct SuperSubclass
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile T*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (sizeof(T) == sizeof(U)) };
		};

		template <>
		struct SuperSubclass<void, void>
		{
			enum { value = false };
		};

		template <class U>
		struct SuperSubclass<void, U>
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile void*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile void*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (0 == sizeof(U)) };
		};

		template <class T>
		struct SuperSubclass<T, void>
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile T*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (sizeof(T) == 0) };
		};

		template<class T, class U>
		struct SuperSubclassStrict
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile T*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType &&
					!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile U*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (sizeof(T) == sizeof(U)) };
		};

		template<>
		struct SuperSubclassStrict<void, void>
		{
			enum { value = false };
		};

		template<class U>
		struct SuperSubclassStrict<void, U>
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile void*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile void*>::sameType &&
					!::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile U*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (0 == sizeof(U)) };
		};

		template<class T>
		struct SuperSubclassStrict<T, void>
		{
			enum {
				value = (::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile T*>::exists &&
				!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType &&
					!::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
			};

			// Dummy enum to make sure that both classes are fully defined.
			enum { dontUseWithIncompleteTypes = (sizeof(T) == 0) };
		};

	} //namespace Loki

	  /*
	  #define LOKI_SUPERSUBCLASS(T, U) \
	  ::_JsCPPUtils_private::Loki::SuperSubclass<T,U>::value

	  #define LOKI_SUPERSUBCLASS_STRICT(T, U) \
	  ::_JsCPPUtils_private::Loki::SuperSubclassStrict<T,U>::value
	  */
}

#endif /* __JSCPPUTILS_PRIVATE_LOKI_SUPERCLASSCHECKER */

/******************** Loki end ********************/


namespace _JsCPPUtils_private
{
	class SmartPointerRootManager;
}

namespace JsCPPUtils
{
	class SmartPointerRefCounter
	{
	private:
		friend class ::_JsCPPUtils_private::SmartPointerRootManager;
		JsCPPUtils::AtomicNum<int> refcnt;
		JsCPPUtils::AtomicNum<int> weakcount;
	public:
		SmartPointerRefCounter() : refcnt(0) { }
		virtual ~SmartPointerRefCounter() { }
	};
}

namespace _JsCPPUtils_private
{
	class SmartPointerRootManager
	{
	public:
		JsCPPUtils::SmartPointerRefCounter refcounter;
		JsCPPUtils::SmartPointerRefCounter *prefcounter;
		JsCPPUtils::AtomicNum<int> *prefcnt;
		JsCPPUtils::AtomicNum<int> *pweakcount;

		void *bkptr;
		SmartPointerRootManager(void *ptr, JsCPPUtils::SmartPointerRefCounter &_refcounter) : bkptr(ptr) {
			this->prefcnt = &_refcounter.refcnt;
			this->pweakcount = &_refcounter.weakcount;
		}
		virtual void destroy() = 0;
		virtual ~SmartPointerRootManager() {};
		void setRefCntPtr(::JsCPPUtils::SmartPointerRefCounter *_prefcounter)
		{
			this->prefcounter = _prefcounter;
			this->prefcnt = &_prefcounter->refcnt;
			this->pweakcount = &_prefcounter->weakcount;
		}
	};

	template <class U, class Deleter>
	class SmartPointerRootManagerImpl : public SmartPointerRootManager
	{
	private:
		U *ptr;
		Deleter d;

	public:
		SmartPointerRootManagerImpl(U *_ptr, Deleter _deleter) :
			SmartPointerRootManager(_ptr, refcounter)
		{
			this->ptr = _ptr;
			this->d = _deleter;

			if (::_JsCPPUtils_private::Loki::SuperSubclassStrict< ::JsCPPUtils::SmartPointerRefCounter, U>::value)
			{
				setRefCntPtr((::JsCPPUtils::SmartPointerRefCounter*)_ptr);
			}

			this->prefcnt->incget();
		}

		void destroy() {
			d(this->ptr);
			this->ptr = NULL;
			this->bkptr = NULL;
		}

		virtual ~SmartPointerRootManagerImpl() { }
	};

	class SmartPointerBase
	{
	protected:
		class FloatingObject {
		public:
			SmartPointerRootManager *m_rootManager;
			void* m_ptr;

			FloatingObject(SmartPointerRootManager *rootManager, void *ptr) :
				m_rootManager(rootManager), m_ptr(ptr) {}
			~FloatingObject(){}
		};

		SmartPointerRootManager *m_rootManager;
		void* m_ptr;

		void _constructor()
		{
			m_rootManager = NULL;
			m_ptr = NULL;
		}

		void copyFrom(const SmartPointerBase &_ref);

		template <class T, class U>
		void copyFromTU(const SmartPointerBase &_ref)
		{
			U * pDerived(reinterpret_cast<U *>(4));
			T * pBase(pDerived);
			size_t offset = reinterpret_cast<intptr_t>(pBase) - reinterpret_cast<intptr_t>(pDerived);

			m_rootManager = _ref.m_rootManager;
			m_ptr = ((char*)_ref.m_ptr + offset);
		}

		int addWeakRef();
		int delWeakRef(bool isSelfDestroy = false);

	public:
		int addRef();
		int delRef(bool isSelfDestroy = false);

		virtual void *detach()
		{
			if (addRef() == 0)
				return NULL;
			return new FloatingObject(m_rootManager, m_ptr);
		}
		virtual void attach(void *ptr)
		{
			FloatingObject *fobj = (FloatingObject*)ptr;
			delRef();
			_constructor();
			m_rootManager = (SmartPointerRootManager*)fobj->m_rootManager;
			m_ptr = fobj->m_ptr;
			delete fobj;
		}

		int getRefCount();

		void reset()
		{
			delRef();
			_constructor();
		}
	};
}

namespace JsCPPUtils
{
	template <class U>
	class DefaultDeleter
	{
	public:
		void operator()(U *ptr) const
		{
			delete ptr;
		}
	};

	template <class T>
		class SmartPointer : public ::_JsCPPUtils_private::SmartPointerBase
		{
		private:
			typedef::_JsCPPUtils_private::SmartPointerBase inherited;
			T *ptr;

			template <class U, class Deleter>
			void setPointer(U* ptr, Deleter d)
			{
				if (ptr == this->m_ptr)
					return;

				if (!ptr && ((char*)this > (char*)this->m_ptr) && (((char*)this->m_ptr + sizeof(T)) > (char*)this))
				{
					if(delRef(true) == 0)
						return;
				} else {
					delRef();
				}
				_constructor();
				if (ptr)
				{
					U * pDerived(reinterpret_cast<U *>(4));
					T * pBase(pDerived);
					size_t offset = reinterpret_cast<intptr_t>(pBase) - reinterpret_cast<intptr_t>(pDerived);

					m_rootManager = new ::_JsCPPUtils_private::SmartPointerRootManagerImpl<U, Deleter>(ptr, d);
					m_ptr = ((char*)ptr + offset);
					this->ptr = (T*)m_ptr;
				}
			}

		public:
			explicit SmartPointer()
			{
				_constructor();
			}

			template<class U, class Deleter>
			explicit SmartPointer(U* ptr, Deleter d)
			{
				_constructor();
				setPointer<U, Deleter>(ptr, d);
			}

			template<class U>
			SmartPointer(U* ptr)
			{
				_constructor();
				setPointer<U, DefaultDeleter<U> >(ptr, DefaultDeleter<U>());
			}

			SmartPointer(int maybeNull)
			{
				_constructor();
			}

			template<class U>
			void operator=(U* ptr)
			{
				setPointer<U, DefaultDeleter<U> >(ptr, DefaultDeleter<U>());
			}

			void operator=(T* ptr)
			{
				setPointer<T, DefaultDeleter<T> >(ptr, DefaultDeleter<T>());
			}

			SmartPointer(const SmartPointer& _ref)
			{
				copyFrom(_ref);
				this->ptr = (T*)m_ptr;
				addRef();
			}

			template<class U>
			SmartPointer(const SmartPointer<U>& _ref)
			{
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addRef();
			}

			template<class U>
			void operator=(const SmartPointer<U>& _ref)
			{
				delRef();
				_constructor();
				copyFromTU<T, U>(_ref);
				this->ptr = (T*)m_ptr;
				addRef();
			}

			void operator=(const SmartPointer<T>& _ref)
			{
				delRef();
				_constructor();
				copyFrom(_ref);
				this->ptr = (T*)m_ptr;
				addRef();
			}

#if (__cplusplus >= 201103) || (defined(HAS_MOVE_SEMANTICS) && HAS_MOVE_SEMANTICS == 1)
			explicit SmartPointer(SmartPointer&& _ref)
			{
				m_ptr = _ref->m_ptr;
				this->ptr = (T*)m_ptr;
				m_rootManager = _ref->m_rootManager;
				_ref->m_ptr = NULL;
				_ref->m_rootManager = NULL;
			}
#endif
				
			~SmartPointer()
			{
				delRef();
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

			void attach(void *ptr)
			{
				inherited::attach(ptr);
				this->ptr = (T*)m_ptr;
			}
		};
}

#endif /* __JSCPPUTILS_SMARTPOINTER_H__ */
