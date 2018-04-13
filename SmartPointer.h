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
				enum {exists = sizeof(typename H::Small) == sizeof((H::Test(H::MakeT())))};
				enum {exists2Way = exists && Conversion < U, T > ::exists};
				enum {sameType = false};
			};

		template <class T>
			struct Conversion<T, T>    
			{
				enum {exists = 1, exists2Way = 1, sameType = 1};
			};

		template <class T>
			struct Conversion<void, T>    
			{
				enum {exists = 0, exists2Way = 0, sameType = 0};
			};

		template <class T>
			struct Conversion<T, void>    
			{
				enum {exists = 0, exists2Way = 0, sameType = 0};
			};

		template <>
			struct Conversion<void, void>    
			{
			public:
				enum {exists = 1, exists2Way = 1, sameType = 1};
			};
	
	
		template <class T, class U>
			struct SuperSubclass
			{
				enum {value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile T*>::exists &&
							  !::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
				};

									  // Dummy enum to make sure that both classes are fully defined.
				enum{dontUseWithIncompleteTypes = (sizeof(T) == sizeof(U))};
			};

		template <>
			struct SuperSubclass<void, void> 
			{
				enum {value = false};
			};

		template <class U>
			struct SuperSubclass<void, U> 
			{
				enum {value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile void*>::exists &&
							  !::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile void*>::sameType)
				};

									  // Dummy enum to make sure that both classes are fully defined.
				enum{dontUseWithIncompleteTypes = (0 == sizeof(U))};
			};

		template <class T>
			struct SuperSubclass<T, void> 
			{
				enum {value = (::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile T*>::exists &&
							  !::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
				};

									  // Dummy enum to make sure that both classes are fully defined.
				enum{dontUseWithIncompleteTypes = (sizeof(T) == 0)};
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
				enum{dontUseWithIncompleteTypes = (sizeof(T) == sizeof(U))};
			};

		template<>
			struct SuperSubclassStrict<void, void> 
			{
				enum {value = false};
			};

		template<class U>
			struct SuperSubclassStrict<void, U> 
			{
				enum {value = (::_JsCPPUtils_private::Loki::Conversion<const volatile U*, const volatile void*>::exists &&
							 !::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile void*>::sameType &&
							 !::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile U*>::sameType)
				};

									 // Dummy enum to make sure that both classes are fully defined.
				enum{dontUseWithIncompleteTypes = (0 == sizeof(U))};
			};

		template<class T>
			struct SuperSubclassStrict<T, void> 
			{
				enum {value = (::_JsCPPUtils_private::Loki::Conversion<const volatile void*, const volatile T*>::exists &&
							 !::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType &&
							 !::_JsCPPUtils_private::Loki::Conversion<const volatile T*, const volatile void*>::sameType)
				};

									 // Dummy enum to make sure that both classes are fully defined.
				enum{dontUseWithIncompleteTypes = (sizeof(T) == 0)};
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
	class SmartPointerBase
	{
	protected:
		class RootManager
		{
		public:
			JsCPPUtils::AtomicNum<int> refcnt;
			RootManager() : refcnt(1) {}
			virtual void destroy() = 0;
			virtual ~RootManager() {};
		};

		template <class U, class Deleter>
		class RootManagerImpl : public RootManager
		{
		private:
			U *ptr;
			Deleter d;

		public:
			RootManagerImpl(U *_ptr, Deleter _deleter) {
				this->ptr = _ptr;
				this->d = _deleter;
			}

			void destroy() {
				d(this->ptr);
				this->ptr = NULL;
			}

			virtual ~RootManagerImpl() { }
		};

		template <class U>
		class DefaultDeleter
		{
		public:
			void operator()(U *ptr) const
			{
				delete ptr;
			}
		};

		RootManager *m_rootManager;
		void* m_ptr;

		void copyFrom(const SmartPointerBase &_ref)
		{
			m_rootManager = _ref.m_rootManager;
			m_ptr = _ref.m_ptr;
		}
	};
}

namespace JsCPPUtils
{
	template <class T>
		class SmartPointer : public ::_JsCPPUtils_private::SmartPointerBase
		{
		private:
			void _constructor()
			{
				m_rootManager = NULL;
				m_ptr = NULL;
			}

			template <class U, class Deleter>
			void setPointer(U* ptr, Deleter d)
			{
				delRef();
				_constructor();
				if (ptr)
				{
					m_rootManager = new RootManagerImpl<U, Deleter>(ptr, d);
					m_ptr = ptr;
				}
			}

		public:
			explicit SmartPointer()
			{
				_constructor();
			}

			int addRef()
			{
				if (m_rootManager)
				{
					return m_rootManager->refcnt.incget();
				}
				return 0;
			}

			int delRef()
			{
				if (m_rootManager)
				{
					int refcnt = m_rootManager->refcnt.decget();
					if (refcnt == 0)
					{
						m_rootManager->destroy();
						delete m_rootManager;
						m_rootManager = NULL;
						m_ptr = NULL;
					}
				}
				return 0;
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
				addRef();
			}

			template<class U>
			SmartPointer(const SmartPointer<U>& _ref)
			{
				copyFrom(_ref);
				addRef();
			}

			template<class U>
			void operator=(const SmartPointer<U>& _ref)
			{
				delRef();
				_constructor();
				copyFrom(_ref);
				addRef();
			}

#if (__cplusplus >= 201103) || (defined(HAS_MOVE_SEMANTICS) && HAS_MOVE_SEMANTICS == 1)
			explicit SmartPointer(SmartPointer&& _ref)
			{
				m_ptr = _ref->m_ptr;
				m_rootManager = _ref->m_rootManager;
				_ref->m_ptr = NULL;
				_ref->m_rootManager = NULL;
			}
#endif
				
			~SmartPointer()
			{
				delRef();
			}

			int getRefCnt()
			{
				if (m_rootManager)
				{
					return m_rootManager->refcnt.get();
				}
				return 0;
			}

			T* operator->() const { return m_ptr; }
			T& operator*() const { return *m_ptr; }
		};
}

#endif /* __JSCPPUTILS_SMARTPOINTER_H__ */
