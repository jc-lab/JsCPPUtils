/**
 * @file	Timer.h
 * @class	Timer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/11/08
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_TIMER_H__
#define __JSCPPUTILS_TIMER_H__

#include "Common.h"
#include "SmartPointer.h"
#include "Thread.h"
#include "Lockable.h"

#include <list>

namespace JsCPPUtils {

	class TimerTask;
	class Timer
	{
	private:
		class WorkerThread;

		enum ScheduleType {
			SCHTYPE_FIXEDDELAY,
			SCHTYPE_FIXEDRATE,
		};
		struct TimerTaskInfo {
			int64_t period;
			int64_t timetostart;
			int64_t lastexecutedtick;
			JsCPPUtils::SmartPointer<TimerTask> task;
			ScheduleType schType;
		};
		std::list<TimerTaskInfo> m_timerTaskQueue;
		Lockable m_timerTaskQueueLock;
		int m_minDelayTime;

		JsCPPUtils::SmartPointer<WorkerThread> m_thread;

	public:
		Timer();
		virtual ~Timer();
		int64_t currentTimeMillis();
		void cancel();
		int purge();
		void schedule(SmartPointer<TimerTask> task, int64_t delay);
		void schedule(SmartPointer<TimerTask> task, int64_t delay, int64_t period);
		void scheduleAtFixedRate(SmartPointer<TimerTask> task, int64_t delay, int64_t period);

		void setMinDelayTime(int minDelayTime);
		int getMinDelayTime();

	private:
		class WorkerThread : public Thread
		{
		public:
			Timer *timer;
			int run(int param_idx, void *param_ptr) override;
		};
	};

}

#endif /* __JSCPPUTILS_TIMER_H__ */
