// VulkanTest1.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Engine.h"
#include <iostream>

using namespace WindGE;

int main()
{
	{
		Application app;
		bool res = app.init();

		if (res)
		{
			std::cout << "vk app init success.." << std::endl;
		}
	}
	system("pause");

    return 0;
}