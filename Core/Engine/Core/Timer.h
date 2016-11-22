#pragma once
#ifndef __TIMER_H__
#define __TIMER_H__

#include "Config.h"

namespace WindGE
{
	class WIND_CORE_API Timer
	{
	public:
		Timer();
		
		float total_time() const;
		float delta_time() const;

		void reset();
		void start();
		void stop();
		void tick();

	private:
		double	__seconds_per_count;
		double	__delta_time;

		__int64	__base_time;
		__int64 __paused_time;
		__int64 __stop_time;
		__int64 __prev_time;
		__int64 __cur_time;
		
		bool	__stopped;
	};
}

#endif // !__TIMER_H__
