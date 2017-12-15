#pragma once
#ifndef __LOG_H__
#define __LOG_H__

#include "Config.h"

namespace WindGE
{
	class WIND_CORE_API Log
	{
	public:
		static void init();
		static void destroy();

		static void error(wchar_t const* fmt, ...);
		static void warning(wchar_t const* fmt, ...);
		static void info(wchar_t const* fmt, ...);

		static void error(char const* fmt, ...);
		static void warning(char const* fmt, ...);
		static void info(char const* fmt, ...);

	private:
		static FILE* __file;
	};
}

#endif // __LOG_H__
