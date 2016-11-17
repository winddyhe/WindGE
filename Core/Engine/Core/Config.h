#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(DEBUG) | defined(_DEBUG)
#define WIND_DEBUG
#endif

#define WIND_CORE_API __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4251)	//disable warning 4251
#pragma warning(disable:4996)	//diasble warning 4996

#endif // __CONFIG_H__
