/**
 * @file	LockableEx.h
 * @class	LockableEx
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/10/14
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_LOCKABLEEX_H__
#define __JSCPPUTILS_LOCKABLEEX_H__

#include "Lockable.h"
#include "HashMap.h"

namespace JsCPPUtils
{
	class LockableEx
	{
	private:
		Lockable m_lock;
		Lockable *m_plockref;
#if defined(JSCUTILS_OS_WINDOWS)
		HashMap<DWORD, int> m_tinfo;
#elif defined(JSCUTILS_OS_LINUX)
		HashMap<pthread_t, int> m_tinfo;
#endif

	public:
		LockableEx(Lockable *plockref = NULL);
		~LockableEx();
		int lock();
		int unlock(bool earseinmap = false);
	};
}

#endif
