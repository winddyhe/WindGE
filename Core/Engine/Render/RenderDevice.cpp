#include "RenderDevice.h"

using namespace WindGE;

RenderDevice::RenderDevice() :
	__window_instance(nullptr),
	__window_hwnd(nullptr),
	__app_short_name("App"),
	__queue_family_count(0),
	__queue_family_index(0),
	__graphics_queue_family_index(0),
	__present_queue_family_index(0)
{
}

RenderDevice::~RenderDevice() 
{
}

bool RenderDevice::init(HINSTANCE inst, HWND hwnd)
{
	__window_instance = inst;
	__window_hwnd = hwnd;

	if (_init_global_layer_properties())  return false;
	if (_init_instance_extension_names()) return false;
	if (_init_device_extension_names())   return false;
	if (_init_instance())				  return false;
	if (!_init_enumerate_device())	      return false;
	if (!_init_surface_khr())			  return false;
	if (_init_device())					  return false;

	return true;
}

void RenderDevice::release() 
{
	vkDestroyDevice(__vk_device, nullptr);
	vkDestroyInstance(__vk_inst, nullptr);
}

bool RenderDevice::_init_global_extension_properties(LayerProperties& layerProps)
{
	VkExtensionProperties* instExtensions;
	uint32_t instExtensionCount = 0;
	VkResult res = VK_SUCCESS;
	char *layerName = nullptr;

	layerName = layerProps.props.layerName;
	do {
		res = vkEnumerateInstanceExtensionProperties(layerName, &instExtensionCount, nullptr);

		if (res != VK_SUCCESS) return false;
		if (instExtensionCount == 0) false;

		layerProps.extensionProps.resize(instExtensionCount);
		instExtensions = layerProps.extensionProps.data();
		res = vkEnumerateInstanceExtensionProperties(layerName, &instExtensionCount, instExtensions);
	} while (res == VK_INCOMPLETE);

	return res == VK_SUCCESS;
}

bool RenderDevice::_init_global_layer_properties()
{
	uint32_t instanceLayerCount = 0;
	VkLayerProperties *vkProps = nullptr;
	VkResult res = VK_SUCCESS;

	do {
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

		if (res != VK_SUCCESS) return false;
		if (instanceLayerCount == 0) return false;

		vkProps = (VkLayerProperties*)realloc(vkProps, instanceLayerCount * sizeof(VkLayerProperties));
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, vkProps);
	} while (res == VK_INCOMPLETE);

	for (uint32_t i = 0; i < instanceLayerCount; i++)
	{
		LayerProperties layerProps;
		layerProps.props = vkProps[i];
		if (!_init_global_extension_properties(layerProps))
			return false;
		__layer_props_vec.push_back(layerProps);
	}
	free(vkProps);

	return res == VK_SUCCESS;
}

bool RenderDevice::_init_instance_extension_names()
{
	__inst_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	__inst_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	return true;
}

bool RenderDevice::_init_device_extension_names()
{
	__device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	return true;
}

bool RenderDevice::_init_instance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = __app_short_name.c_str();
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = __app_short_name.c_str();
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pNext = nullptr;
	instInfo.flags = 0;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledExtensionCount = (unsigned int)__inst_extension_names.size();
	instInfo.ppEnabledExtensionNames = __inst_extension_names.size() ? __inst_extension_names.data() : nullptr;
	instInfo.enabledLayerCount = 0;
	instInfo.ppEnabledLayerNames = nullptr;

	// 初始化VkInstance
	VkResult res = vkCreateInstance(&instInfo, nullptr, &__vk_inst);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		Log::error(L"cannot find a compatible Vulkan ICD.");
		return false;
	}
	if (res != VK_SUCCESS)
	{
		Log::error(L"unknown error.");
		return false;
	}
	return true;
}

bool RenderDevice::_init_enumerate_device()
{
	// 枚举物理设备
	uint32_t gpuCount = 1;
	VkResult res = vkEnumeratePhysicalDevices(__vk_inst, &gpuCount, nullptr);
	Log::info(L"gpu count: %u", gpuCount);
	if (res != VK_SUCCESS || gpuCount < 1)
	{
		Log::error(L"not found gpu devices..");
		return false;
	}
	__vk_gpus.resize(gpuCount);
	res = vkEnumeratePhysicalDevices(__vk_inst, &gpuCount, __vk_gpus.data());

	// Device Queue Family Properties
	vkGetPhysicalDeviceQueueFamilyProperties(__vk_gpus[0], &__queue_family_count, nullptr);
	Log::info(L"device queue family prop count: %u", __queue_family_count);
	if (__queue_family_count < 1)
	{
		Log::error(L"not found queue family props..");
		return false;
	}

	__vk_queue_family_props.resize(__queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(__vk_gpus[0], &__queue_family_count, __vk_queue_family_props.data());

	bool isFound = false;
	for (uint32_t i = 0; i < __vk_queue_family_props.size(); ++i)
	{
		if (__vk_queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			__queue_family_index = i;
			isFound = true;
			break;
		}
	}
	if (!isFound)
	{
		Log::error(L"create device failed..");
		return false;
	}

	vkGetPhysicalDeviceMemoryProperties(__vk_gpus[0], &__vk_memory_props);
	vkGetPhysicalDeviceProperties(__vk_gpus[0], &__vk_gpu_props);

	return true;
}

bool RenderDevice::_init_surface_khr()
{
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.hinstance = __window_instance;
	createInfo.hwnd = __window_hwnd;

	VkResult res = vkCreateWin32SurfaceKHR(__vk_inst, &createInfo, nullptr, &__vk_surface);
	if (res != VK_SUCCESS)
	{
		Log::error(L"create win32 surface khr failed..");
		return false;
	}

	VkBool32 *pSupportsPresent = (VkBool32 *)malloc(__queue_family_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < __queue_family_count; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(__vk_gpus[0], i, __vk_surface, &pSupportsPresent[i]);
	}

	__graphics_queue_family_index = UINT32_MAX;
	__present_queue_family_index = UINT32_MAX;

	for (uint32_t i = 0; i < __queue_family_count; ++i)
	{
		if ((__vk_queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (__graphics_queue_family_index == UINT32_MAX)
				__graphics_queue_family_index = i;

			if (pSupportsPresent[i] == VK_TRUE)
			{
				__graphics_queue_family_index = i;
				__present_queue_family_index = i;
				break;
			}
		}
	}

	if (__present_queue_family_index == UINT32_MAX)
	{
		for (size_t i = 0; i < __queue_family_count; ++i)
		{
			if (pSupportsPresent[i] == VK_TRUE)
			{
				__present_queue_family_index = (unsigned int)i;
				break;
			}
		}
	}
	free(pSupportsPresent);

	if (__graphics_queue_family_index == UINT32_MAX || __present_queue_family_index == UINT32_MAX)
	{
		Log::error(L"Could not find a queues for graphics and present.");
		return false;
	}

	uint32_t formatCount = 0;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(__vk_gpus[0], __vk_surface, &formatCount, nullptr);
	if (res != VK_SUCCESS)
	{
		Log::error(L"get physical device surface formats KHR failed.");
		return false;
	}
	Log::info(L"physical device surface formats KHR Count: %u", formatCount);
	VkSurfaceFormatKHR* surfFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(__vk_gpus[0], __vk_surface, &formatCount, surfFormats);
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		__surface_format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		__surface_format = surfFormats[0].format;
	}
	free(surfFormats);

	return true;
}

bool RenderDevice::_init_device()
{
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.queueCount = 1;
	float queuePriorities[1] = { 0.0f };
	queueInfo.pQueuePriorities = queuePriorities;
	queueInfo.queueFamilyIndex = __queue_family_index;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = (unsigned int)__device_extension_names.size();
	deviceInfo.ppEnabledExtensionNames = __device_extension_names.size() ? __device_extension_names.data() : nullptr;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.pEnabledFeatures = nullptr;

	VkResult res = vkCreateDevice(__vk_gpus[0], &deviceInfo, nullptr, &__vk_device);
	if (res != VK_SUCCESS)
	{
		Log::error(L"create device failed..");
		return false;
	}
	return true;
}