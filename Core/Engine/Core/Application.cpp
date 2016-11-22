#include "Application.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace WindGE;

Application::Application() :
	__client_width(0),
	__client_height(0)
{
}

Application::~Application()
{
	VkCommandBuffer cmdBuffers[1] = { __vk_cmd_buffer };
	if (__vk_device && __vk_cmd_pool)
	{
		vkFreeCommandBuffers(__vk_device, __vk_cmd_pool, 1, cmdBuffers);
		vkDestroyCommandPool(__vk_device, __vk_cmd_pool, nullptr);
	}
	
	vkDestroyDevice(__vk_device, nullptr);
	vkDestroyInstance(__vk_inst, nullptr);
}

bool Application::init(HINSTANCE inst, HWND hwnd, int width, int height)
{
	__client_width = width;
	__client_height = height;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "App1";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "App1";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pNext = nullptr;
	instInfo.flags = 0;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledExtensionCount = 0;
	instInfo.ppEnabledExtensionNames = nullptr;
	instInfo.enabledLayerCount = 0;
	instInfo.ppEnabledLayerNames = nullptr;

	VkResult res;

	// 初始化VkInstance
	res = vkCreateInstance(&instInfo, nullptr, &__vk_inst);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		std::cout << "cannot find a compatible Vulkan ICD." << std::endl;
		return false;
	}
	else if (res) 
	{
		std::cout << "unknown error." << std::endl;
		return false;
	}

	// 枚举物理设备
	uint32_t gpuCount = 1;
	res = vkEnumeratePhysicalDevices(__vk_inst, &gpuCount, nullptr);
	std::cout << "gpu count: " << gpuCount << std::endl;
	if (res != VK_SUCCESS || gpuCount < 1)
	{
		std::cout << "not found gpu devices.." << std::endl;
		return false;
	}
	__vk_gpus.resize(gpuCount);
	res = vkEnumeratePhysicalDevices(__vk_inst, &gpuCount, __vk_gpus.data());

	// 初始化设备
    VkDeviceQueueCreateInfo queueInfo = {};

	uint32_t queueFamilyCount = 1;
	vkGetPhysicalDeviceQueueFamilyProperties(__vk_gpus[0], &queueFamilyCount, nullptr);
	std::cout << "device queue family prop count: " << queueFamilyCount << std::endl;
	if (res != 0 || queueFamilyCount < 1)
	{
		std::cout << "not found queue family props.." << std::endl;
		return false;
	}
	__vk_queue_family_props.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(__vk_gpus[0], &queueFamilyCount, __vk_queue_family_props.data());

	bool isFound = false;
	for (uint32_t i = 0; i < __vk_queue_family_props.size(); ++i)
	{
		if (__vk_queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueInfo.queueFamilyIndex = i;
			isFound = true;
			break;
		}
	}
	if (!isFound)
	{
		std::cout << "create device failed.." << std::endl;
		return false;
	}

	float queuePriorities[1] = { 0.0f };
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = 0;
	deviceInfo.ppEnabledExtensionNames = 0;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.pEnabledFeatures = nullptr;

	res = vkCreateDevice(__vk_gpus[0], &deviceInfo, nullptr, &__vk_device);
	if (res != VK_SUCCESS)
	{
		std::cout << "create device failed.." << std::endl;
		return false;
	}

	// 创建Command buffer
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = nullptr;
	cmdPoolInfo.queueFamilyIndex = queueInfo.queueFamilyIndex;
	cmdPoolInfo.flags = 0;

	res = vkCreateCommandPool(__vk_device, &cmdPoolInfo, nullptr, &__vk_cmd_pool);
	if (res != VK_SUCCESS)
	{
		std::cout << "create command pool failed.." << std::endl;
		return false;
	}

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = nullptr;
	cmd.commandPool = __vk_cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;
		
	res = vkAllocateCommandBuffers(__vk_device, &cmd, &__vk_cmd_buffer);
	if (res != VK_SUCCESS)
	{
		std::cout << "create command buffer failed.." << std::endl;
		return false;
	}

	// 构建Surface
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.hinstance = inst;
	surfaceCreateInfo.hwnd = hwnd;
	res = vkCreateWin32SurfaceKHR(__vk_inst, &surfaceCreateInfo, nullptr, &__vk_surface);
	if (res != VK_SUCCESS)
	{
		std::cout << "create surface failed.." << std::endl;
		return false;
	}
	
	return true;
}

void Application::resize(int w, int h)
{
	__client_width = w;
	__client_height = h;
}

void Application::update(float /*deltaTime*/)
{
}

void Application::draw()
{

}