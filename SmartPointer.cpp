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
		m_rootManager = _ref.m_rootManager;
		m_ptr = _ref.m_ptr;
	}

	int SmartPointerBase::addRef()
	{
		if (m_rootManager)
		{
			if (!m_rootManager->bkptr)
				return 0;
			return m_rootManager->prefcnt->incget() - m_rootManager->pweakcount->get();
		}
		return 0;
	}

	int SmartPointerBase::delRef(bool isSelfDestroy)
	{
		SmartPointerRootManager * volatile pRootManager = m_rootManager;
		if (pRootManager)
		{
			int refcnt = pRootManager->prefcnt->decget();
			int weakcnt = pRootManager->pweakcount->get();
			int remaincnt = refcnt - weakcnt;
			if (remaincnt == 0)
			{
				pRootManager->destroy();
				if(refcnt == 0)
					delete pRootManager;
				if (isSelfDestroy)
					return 0;
				m_rootManager = NULL;
				m_ptr = NULL;
			}
			return remaincnt;
		}
		return 0;
	}

	int SmartPointerBase::addWeakRef()
	{
		if (m_rootManager)
		{
			if (!m_rootManager->bkptr)
				return 0;
			return m_rootManager->prefcnt->incget() - m_rootManager->pweakcount->incget();
		}
		return 0;
	}

	int SmartPointerBase::delWeakRef(bool isSelfDestroy)
	{
		SmartPointerRootManager * volatile pRootManager = m_rootManager;
		if (pRootManager)
		{
			int refcnt = pRootManager->prefcnt->decget();
			int weakcnt = pRootManager->pweakcount->decget();
			int remaincnt = refcnt - weakcnt;
			if (remaincnt == 0)
			{
				pRootManager->destroy();
				if (refcnt == 0)
					delete pRootManager;
				if (isSelfDestroy)
					return 0;
				m_rootManager = NULL;
				m_ptr = NULL;
			}
			return remaincnt;
		}
		return 0;
	}

	int SmartPointerBase::getRefCount()
	{
		if (m_rootManager)
		{
			return m_rootManager->prefcnt->get();
		}
		return 0;
	}
}
