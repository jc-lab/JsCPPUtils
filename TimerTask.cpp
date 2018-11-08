/**
 * @file	TimerTask.cpp
 * @class	TimerTask
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/11/08
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "TimerTask.h"

namespace JsCPPUtils {

	TimerTask::TimerTask()
	{
		m_active = true;
		m_scheduledExecutionTime = 0;
	}

	TimerTask::~TimerTask()
	{
	}

	void TimerTask::execute(TimerTask *task, int64_t curtime)
	{
		// TimerTask *task : 그냥 run()호출하면 TimerTask Base Class의 run이 호출됨... 
		if (m_active) {
			m_scheduledExecutionTime = curtime;
			run();
		}
	}

	bool TimerTask::cancel()
	{
		if (m_active)
		{
			m_active = false;
			return true;
		}
		return false;
	}

	int64_t TimerTask::scheduledExecutionTime()
	{
		return m_scheduledExecutionTime;
	}

}
