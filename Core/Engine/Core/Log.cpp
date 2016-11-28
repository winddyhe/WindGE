#include "Log.h"
#include <stdarg.h>
#include <mutex>

using namespace WindGE;

FILE* Log::__file = nullptr;

void Log::init()
{
	AllocConsole();

	fopen_s(&__file, "Log.txt", "a+");
	freopen_s(&__file, ("CONOUT$"), ("w+t"), stderr);
	setlocale(LC_ALL, ("chs"));
}

void Log::destroy() 
{
	fclose(__file);
	FreeConsole();

	__file = nullptr;
}

std::mutex mu_io;
void logPrint(const std::wstring& type, const std::wstring& buf)
{
	mu_io.lock();
	std::wclog << type << buf << std::endl;
	mu_io.unlock();
}

void Log::error(wchar_t const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	wchar_t buf[1024];
	memset(buf, 0, 1024);
	(void)vswprintf(buf, fmt, args);
	va_end(args);
	std::wstring typestr = L"(ERROR) ";
	std::wstring bufstr(buf);
	
	logPrint(typestr, bufstr);
}

void Log::warning(wchar_t const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	wchar_t buf[1024];
	memset(buf, 0, 1024);
	(void)vswprintf(buf, fmt, args);
	va_end(args);
	std::wstring typestr = L"(WARN) ";
	std::wstring bufstr(buf);

	logPrint(typestr, bufstr);
}

void Log::info(wchar_t const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	wchar_t buf[1024];
	memset(buf, 0, 1024);
	(void)vswprintf(buf, fmt, args);
	va_end(args);
	std::wstring typestr = L"(INFO) ";
	std::wstring bufstr;
	bufstr.assign(buf);

	logPrint(typestr, bufstr);
}