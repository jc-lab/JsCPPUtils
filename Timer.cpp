/**
 * @file	Timer.cpp
 * @class	Timer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/11/08
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "Common.h"
#include "Timer.h"
#include "TimerTask.h"

namespace JsCPPUtils {

	Timer::Timer()
	{
		m_minDelayTime = 100;
		m_thread = new WorkerThread();
		m_thread->timer = this;
		m_thread->start();
	}

	Timer::~Timer()
	{
	}

	int64_t Timer::currentTimeMillis()
	{
		return Common::getTickCount();
	}

	void Timer::cancel()
	{
		m_thread->reqStop();
	}

	int Timer::purge()
	{
		return 0;
	}

	void Timer::schedule(SmartPointer<TimerTask> task, int64_t delay)
	{
		TimerTaskInfo info;
		info.timetostart = delay;
		info.period = 0;
		info.task = task;
		info.lastexecutedtick = currentTimeMillis() + delay;
		info.schType = SCHTYPE_FIXEDDELAY;
		m_timerTaskQueueLock.lock();
		m_timerTaskQueue.push_back(info);
		m_timerTaskQueueLock.unlock();
	}

	void Timer::schedule(SmartPointer<TimerTask> task, int64_t delay, int64_t period)
	{
		TimerTaskInfo info;
		info.timetostart = delay;
		info.period = period;
		info.task = task;
		info.lastexecutedtick = currentTimeMillis() + delay;
		info.schType = SCHTYPE_FIXEDDELAY;
		m_timerTaskQueueLock.lock();
		m_timerTaskQueue.push_back(info);
		m_timerTaskQueueLock.unlock();
	}

	void Timer::scheduleAtFixedRate(SmartPointer<TimerTask> task, int64_t delay, int64_t period)
	{
		TimerTaskInfo info;
		info.timetostart = delay;
		info.period = period;
		info.task = task;
		info.lastexecutedtick = currentTimeMillis() + delay;
		info.schType = SCHTYPE_FIXEDRATE;
		m_timerTaskQueueLock.lock();
		m_timerTaskQueue.push_back(info);
		m_timerTaskQueueLock.unlock();
	}

	void Timer::setMinDelayTime(int minDelayTime)
	{
		m_minDelayTime = minDelayTime;
	}

	int Timer::getMinDelayTime()
	{
		return m_minDelayTime;
	}

	int Timer::WorkerThread::run(int param_idx, void *param_ptr)
	{
		while (Thread::isRun())
		{
			int delayTime;

			timer->m_timerTaskQueueLock.lock();
			for (std::list<TimerTaskInfo>::iterator iter = timer->m_timerTaskQueue.begin(); iter != timer->m_timerTaskQueue.end(); iter++)
			{
				int64_t curtime = Common::getTickCount();
				if (iter->timetostart <= curtime)
				{
					iter->timetostart = -1;
					if (iter->schType == SCHTYPE_FIXEDRATE)
						iter->lastexecutedtick = curtime;
					iter->task->execute(iter->task.getPtr(), curtime);
					if (iter->schType == SCHTYPE_FIXEDDELAY)
						iter->lastexecutedtick = Common::getTickCount();
				}
				else {
					int64_t exectime = (iter->lastexecutedtick + iter->period);
					if (exectime <= curtime)
					{
						if (iter->schType == SCHTYPE_FIXEDRATE)
							iter->lastexecutedtick = curtime;
						iter->task->execute(iter->task.getPtr(), curtime);
						if (iter->schType == SCHTYPE_FIXEDDELAY)
							iter->lastexecutedtick = Common::getTickCount();
					}
				}
			}
			timer->m_timerTaskQueueLock.unlock();

			delayTime = timer->m_minDelayTime;
			Sleep(delayTime);
		}
		return 0;
	}
}
