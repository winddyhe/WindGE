// VulkanTest1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Engine.h"
#include <iostream>

using namespace WindGE;

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE /*hPrevInstance*/, __in LPSTR /*lpCmdLine*/, __in int /*nShowCmd*/)
{
	{
		Log::init();

		Win32Window window(hInstance);

		Application app;
		bool res = window.init(&app, L"AppTest");

		if (!res)
		{
			Log::error(L"vk app init faield..");
			return 0;
		}
		return window.run();
		
		Log::destroy();
	}
	return 0;
}