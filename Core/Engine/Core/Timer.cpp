#include "Timer.h"

using namespace WindGE;

Timer::Timer(void) :
	__seconds_per_count(0.0),
	__delta_time(-1.0),
	__base_time(0),
	__paused_time(0),
	__prev_time(0),
	__cur_time(0),
	__stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	__seconds_per_count = 1.0 / (double)countsPerSec;
}

float Timer::total_time() const
{
	if (__stopped)
	{
		return (float)(((__stop_time - __paused_time) - __base_time) * __seconds_per_count);
	}
	else
	{
		return (float)(((__cur_time - __paused_time) - __base_time) * __seconds_per_count);
	}
}

float Timer::delta_time() const
{
	return (float)__delta_time;
}

void Timer::reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	__base_time = currTime;
	__prev_time = currTime;
	__stop_time = 0;
	__stopped = false;
}

void Timer::start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (__stopped)
	{
		__paused_time += (startTime - __stop_time);
		__prev_time = startTime;
		__stop_time = 0;
		__stopped = false;
	}
}

void Timer::stop()
{
	if (!__stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		__stop_time = currTime;
		__stopped = true;
	}
}

void Timer::tick()
{
	if (__stopped)
	{
		__delta_time = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	__cur_time = currTime;

	__delta_time = (__cur_time - __prev_time) * __seconds_per_count;

	__prev_time = __cur_time;

	if (__delta_time < 0.0)
	{
		__delta_time = 0.0;
	}
}