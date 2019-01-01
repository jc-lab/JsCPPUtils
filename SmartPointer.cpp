/**
 * @file	SmartPointer.cpp
 * @class	SmartPointer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/04/16
 * @brief	SmartPointer (thread-safe). It can be used as a smart object by inheritance.
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */
#include "SmartPointer.h"

namespace _JsCPPUtils_private
{

	void SmartPointerBase::copyFrom(const SmartPointerBase &_ref)
	{
		m_refcounter = _ref.m_refcounter;
		m_rootManager = _ref.m_rootManager;
		m_ptr = _ref.m_ptr;
	}

	int SmartPointerBase::addRef()
	{
		if (m_refcounter)
		{
			if (m_refcounter->alive.get() == 1)
				return m_refcounter->refcnt.incget() - m_refcounter->weakcount.get();
		}
		return 0;
	}

	int SmartPointerBase::delRef(bool isSelfDestroy)
	{
		SmartPointerRootManager * volatile pRootManager = m_rootManager;
		::_JsCPPUtils_private::SmartPointerRefCounterObject * volatile pRefcounter = m_refcounter;
		if (pRefcounter)
		{
			int refcnt = pRefcounter->refcnt.decget();
			int weakcnt = pRefcounter->weakcount.get();
			int remaincnt = refcnt - weakcnt;
			if (remaincnt == 0)
			{
				bool alive = pRefcounter->alive.get() == 1;
				if (alive && pRootManager)
				{
					pRefcounter->alive.set(0);
					pRootManager->destroy();
					delete pRootManager;
				}
				if (weakcnt == 0)
				{
					delete pRefcounter;
					pRefcounter = NULL;
				}
				if (isSelfDestroy)
					return 0;
				m_rootManager = NULL;
				m_ptr = NULL;
				if (weakcnt == 0)
					m_refcounter = NULL;
			}
			return remaincnt;
		}
		return 0;
	}

	int SmartPointerBase::addWeakRef()
	{
		if (m_refcounter)
		{
			if(m_refcounter->alive.get() == 1)
				return m_refcounter->refcnt.incget() - m_refcounter->weakcount.incget();
		}
		return 0;
	}

	int SmartPointerBase::delWeakRef(bool isSelfDestroy)
	{
		SmartPointerRootManager * volatile pRootManager = m_rootManager;
		::_JsCPPUtils_private::SmartPointerRefCounterObject * volatile pRefcounter = m_refcounter;
		if (pRootManager && pRefcounter)
		{
			int refcnt = pRefcounter->refcnt.decget();
			int weakcnt = pRefcounter->weakcount.decget();
			int remaincnt = refcnt - weakcnt;
			if (remaincnt == 0)
			{
				bool alive = pRefcounter->alive.get() == 1;
				if (alive && pRootManager)
				{
					pRefcounter->alive.set(0);
					pRootManager->destroy();
					delete pRootManager;
				}
				if (weakcnt == 0)
				{
					delete pRefcounter;
					pRefcounter = NULL;
				}
				if (isSelfDestroy)
					return 0;
				m_rootManager = NULL;
				m_ptr = NULL;
				if (weakcnt == 0)
					m_refcounter = NULL;
			}
			return remaincnt;
		}
		return 0;
	}

	int SmartPointerBase::getRefCount()
	{
		if (m_refcounter)
		{
			return m_refcounter->refcnt.get();
		}
		return 0;
	}
}
