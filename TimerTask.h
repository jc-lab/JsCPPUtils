/**
 * @file	TimerTask.h
 * @class	TimerTask
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/11/08
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_TIMERTASK_H__
#define __JSCPPUTILS_TIMERTASK_H__

#include "Common.h"

namespace JsCPPUtils {

	class Timer;
	class TimerTask
	{
	private:
		friend class Timer;
		bool m_active;
		int64_t m_scheduledExecutionTime;

		void execute(TimerTask *task, int64_t curtime);

	public:
		TimerTask();
		virtual ~TimerTask();
		bool cancel();
		virtual void run() = 0;
		int64_t scheduledExecutionTime();
	};

}

#endif /* __JSCPPUTILS_TIMERTASK_H__ */
