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

#include "Lockable.h"

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

namespace JsCPPUtils
{
	template <class T>
		class SmartPointer {
		private:
			unsigned int m_isRoot;
			T *m_ptr;
			SmartPointer<T> *m_root_smartptr;

			Lockable *m_pLock;
			int m_refCount;

			int m_isDeleted;
			
			void setPointerSamePtr(T* p, bool isinit = false)
			{
				if (!isinit)
				{
					if (m_root_smartptr != NULL)
					{
						m_root_smartptr->delRef();
						// m_root_smartptr = NULL;
					}
				}
				
				if (p)
				{
					if (::_JsCPPUtils_private::Loki::SuperSubclassStrict<SmartPointer<T>, T >::value)
						m_root_smartptr = (SmartPointer<T>*)p;
					else
						m_root_smartptr = new SmartPointer<T>(p, true);
					m_ptr = p;
					m_root_smartptr->addRef();
				}else{
					m_ptr = NULL;
					m_root_smartptr = NULL;
				}
			}

			void setPointerOtherPtr(T* p, bool isinit = false)
			{
				if (!isinit)
				{
					if (m_root_smartptr != NULL)
					{
						m_root_smartptr->delRef();
						// m_root_smartptr = NULL;
					}
				}

				if (p)
				{
					if (::_JsCPPUtils_private::Loki::SuperSubclassStrict<SmartPointer<T>, T >::value)
						m_root_smartptr = (SmartPointer<T>*)p;
					else
					{
						throw std::exception("This pointer is NOT SubClass for SmartPointer<T>");
					}
					m_ptr = p;
					m_root_smartptr->addRef();
				} else {
					m_ptr = NULL;
					m_root_smartptr = NULL;
				}
			}

		protected:
			virtual void OnPreRelease()
			{
			}

			explicit SmartPointer(T* p, bool isRoot)
				: m_pLock(NULL)
				, m_refCount(0)
			{
				m_isRoot = isRoot ? 1 : 0;
				if (isRoot)
				{
					m_ptr = p;
					m_root_smartptr = NULL;
					m_pLock = new Lockable();
				} else {
					setPointer(p, true);
				}
			}

			explicit SmartPointer(const SmartPointer<T>* pRefObj)
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
				m_root_smartptr = pRefObj->m_root_smartptr;
				if (m_root_smartptr != NULL)
				{
					m_ptr = m_root_smartptr->m_ptr;
					m_root_smartptr->addRef();
				}
			}

		public:
			void addRef()
			{
				if (m_isRoot == 1)
				{
					m_pLock->lock();
					m_refCount++;
					m_pLock->unlock();
				}else if(m_isRoot == 0){
					m_root_smartptr->addRef();
				}else{
					printf("BUG DETECTED!\n");
				}
			}

			/**
			 * @return	Returns true if the reference no longer exists and is released.
			 *			Otherwise, returns false.
			 */
			bool delRef()
			{
				if (m_isRoot == 1)
				{
					m_pLock->lock();
					m_refCount--;
					if (m_refCount == 0)
					{
						if ((void*)m_ptr == (void*)this)
						{
							OnPreRelease();
							m_isDeleted = 0xFFFF5555;
							delete m_ptr;
						} else {
							if (m_ptr != NULL)
								delete m_ptr;
							delete this;
						}
						return true;
					}else if(m_refCount < 0)
					{
						printf("BUG DETECTED!\n");
						assert(m_isRoot >= 0);
					}
					m_pLock->unlock();
					return false;
				}else if(m_isRoot == 0){
					if (m_root_smartptr != NULL)
						m_root_smartptr->delRef();
					return false;
				}else{
					printf("BUG DETECTED!\n");
					assert(m_isRoot >= 0);
				}
				return false;
			}
			
			int getRefCount()
			{
				if (m_root_smartptr != NULL)
					return m_root_smartptr->getRefCount();
				else
					return m_refCount;
			}

		public:
			explicit SmartPointer()
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
			}
		
			template <typename U>
			SmartPointer(U* p)
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
				setPointerOtherPtr(p, true);
			}

			SmartPointer(T* p)
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
				setPointerSamePtr(p, true);
			}

			// Copy constructor.
			SmartPointer(const SmartPointer<T>& refObj)
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
				m_root_smartptr = refObj.m_root_smartptr;
				if (m_root_smartptr != NULL)
				{
					m_ptr = m_root_smartptr->m_ptr;
					m_root_smartptr->addRef();
				}
			}

#if (__cplusplus >= 201103) || (defined(HAS_MOVE_SEMANTICS) && HAS_MOVE_SEMANTICS == 1)
			// Move constructor
			explicit SmartPointer(SmartPointer<T> &&refObj)
				: m_isRoot(0)
				, m_ptr(NULL)
				, m_root_smartptr(NULL)
				, m_pLock(NULL)
				, m_refCount(0)
			{
				m_root_smartptr = refObj.m_root_smartptr;
				if (m_root_smartptr != NULL)
				{
					m_ptr = m_root_smartptr->m_ptr;
					m_root_smartptr->addRef();
				}
			}
#endif

			~SmartPointer()
			{
				if (m_isRoot == 0)
				{
					delRef();
				}else if(m_isRoot != 1)
				{
					printf("BUG DETECTED!\n");
				}
				if (m_pLock != NULL)
				{
					delete m_pLock;
					m_pLock = NULL;
				}
			}

			T* operator->() const
			{
				return m_ptr;
			}

			/*
			T& operator *() const
			{
				return *m_ptr;
			}
			*/

			T* getPtr() const
			{
				return m_ptr;
			}

			template <typename U>
			SmartPointer<T>& operator=(U* p)
			{
				setPointerOtherPtr(p);
				return *this;
			}

			SmartPointer<T>& operator=(T* p)
			{
				setPointerSamePtr(p);
				return *this;
			}
			
			/*
			SmartPointer<T>& operator=(const SmartPointer<T>& refObj)
			{
			if(m_root_smartptr != NULL)
			m_root_smartptr->delRef();
			m_root_smartptr = refObj.m_root_smartptr;
			if(m_root_smartptr != NULL)
			{
			m_ptr = m_root_smartptr->m_ptr;
			m_root_smartptr->addRef();
			}
			return *this;
			}
			*/

			void operator=(const SmartPointer<T>& refObj)
			{
				if (m_root_smartptr != NULL)
					m_root_smartptr->delRef();
				m_root_smartptr = refObj.m_root_smartptr;
				if (m_root_smartptr != NULL)
				{
					m_ptr = m_root_smartptr->m_ptr;
					m_root_smartptr->addRef();
				}
			}

			bool operator!() const
			{
				return (m_ptr == NULL);
			}

			bool operator==(SmartPointer<T> p) const
			{
				return m_ptr == p.m_ptr;
			}

			bool operator!=(SmartPointer<T> p) const
			{
				return m_ptr != p.m_ptr;
			}

			bool operator==(T* p) const
			{
				return m_ptr == p;
			}

			bool operator!=(T* p) const
			{
				return m_ptr != p;
			}

			SmartPointer<T> *detach()
			{
				if (m_root_smartptr != NULL)
					m_root_smartptr->addRef();
				return m_root_smartptr;
			}

			void attach(SmartPointer<T> *p)
			{
				if (m_root_smartptr != NULL)
					m_root_smartptr->delRef();
				m_root_smartptr = p;
				if (m_root_smartptr != NULL)
					m_ptr = m_root_smartptr->m_ptr;
			}
		};
}

#endif /* __JSCPPUTILS_SMARTPOINTER_H__ */
