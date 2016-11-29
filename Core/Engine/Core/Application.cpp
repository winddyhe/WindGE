#include "Application.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace WindGE;

Application::Application() :
	__client_width(0),
	__client_height(0),
	__window_instance(nullptr),
	__window_hwnd(nullptr),
	__app_short_name("App"),
	__queue_family_count(0),
	__queue_family_index(0),
	__graphics_queue_family_index(0),
	__present_queue_family_index(0),
	__swapchain_image_count(0),
	__current_swapchain_buffer(0)
{
}

Application::~Application()
{	
	if (__vk_device)
	{
		for (uint32_t i = 0; i < __swapchain_image_count; i++) 
		{
			vkDestroyImageView(__vk_device, __vk_swapchain_buffers[i].view, NULL);
		}
		vkDestroySwapchainKHR(__vk_device, __vk_swapchain, NULL);

		VkCommandBuffer cmdBufs[1] = { __vk_command_buffer };
		vkFreeCommandBuffers(__vk_device, __vk_commnad_pool, 1, cmdBufs);
		vkDestroyCommandPool(__vk_device, __vk_commnad_pool, nullptr);
	}
	vkDestroyDevice(__vk_device, nullptr);
	vkDestroyInstance(__vk_inst, nullptr);
}

bool Application::init(HINSTANCE inst, HWND hwnd, int width, int height)
{
	__window_instance = inst;
	__window_hwnd     = hwnd;
	__client_width    = width;
	__client_height   = height;

	if (_init_global_layer_properties())  return false;
	if (_init_instance_extension_names()) return false;
	if (_init_device_extension_names())   return false;
	if (_init_instance())				  return false;
	if (!_init_enumerate_device())	      return false;
	if (!_init_surface_khr())			  return false;
	if (_init_device())					  return false;

	if (_init_command_pool())			  return false;
	if (_init_command_buffer())			  return false;
	if (_execute_begin_command_buffer())  return false;
	
	_init_device_queue();

	if (_init_swap_chain())				  return false;

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

VkResult Application::_init_global_extension_properties(LayerProperties& layerProps)
{
	VkExtensionProperties* instExtensions;
	uint32_t instExtensionCount = 0;
	VkResult res;
	char *layerName = nullptr;

	layerName = layerProps.props.layerName;
	do {
		res = vkEnumerateInstanceExtensionProperties(layerName, &instExtensionCount, nullptr);
		
		if (res) return res;
		if (instExtensionCount == 0) res;
		
		layerProps.extensionProps.resize(instExtensionCount);
		instExtensions = layerProps.extensionProps.data();
		res = vkEnumerateInstanceExtensionProperties(layerName, &instExtensionCount, instExtensions);
	} while (res == VK_INCOMPLETE);
	
	return res;
}

VkResult Application::_init_global_layer_properties()
{
	uint32_t instanceLayerCount = 0;
	VkLayerProperties *vkProps = nullptr;
	VkResult res;

	do {
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		
		if (res) return res;
		if (instanceLayerCount == 0) return res;

		vkProps = (VkLayerProperties*)realloc(vkProps, instanceLayerCount*sizeof(VkLayerProperties));
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, vkProps);
	} while (res == VK_INCOMPLETE);

	for (uint32_t i = 0; i < instanceLayerCount; i++)
	{
		LayerProperties layerProps;
		layerProps.props = vkProps[i];
		res = _init_global_extension_properties(layerProps);
		if (res) return res;
		__layer_props_vec.push_back(layerProps);
	}
	free(vkProps);

	return res;
}

VkResult Application::_init_instance_extension_names()
{
	__inst_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	__inst_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	return VK_SUCCESS;
}

VkResult Application::_init_device_extension_names()
{
	__device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	return VK_SUCCESS;
}

VkResult Application::_init_instance()
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
	instInfo.enabledExtensionCount = __inst_extension_names.size();
	instInfo.ppEnabledExtensionNames = __inst_extension_names.size() ? __inst_extension_names.data() : nullptr;
	instInfo.enabledLayerCount = 0;
	instInfo.ppEnabledLayerNames = nullptr;

	// 初始化VkInstance
	VkResult res = vkCreateInstance(&instInfo, nullptr, &__vk_inst);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		Log::error(L"cannot find a compatible Vulkan ICD.");
	}
	else if (res)
	{
		Log::error(L"unknown error.");
	}
	return res;
}

bool Application::_init_enumerate_device()
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
	return true;
}

bool Application::_init_surface_khr()
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
				__present_queue_family_index = i;
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

VkResult Application::_init_device()
{
	// 初始化设备
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
	deviceInfo.enabledExtensionCount = __device_extension_names.size();
	deviceInfo.ppEnabledExtensionNames = __device_extension_names.size() ? __device_extension_names.data() : nullptr;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.pEnabledFeatures = nullptr;

	VkResult res = vkCreateDevice(__vk_gpus[0], &deviceInfo, nullptr, &__vk_device);
	if (res != VK_SUCCESS)
	{
		Log::error(L"create device failed..");
	}
	return res;
}

VkResult Application::_init_command_pool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = nullptr;
	cmdPoolInfo.queueFamilyIndex = __graphics_queue_family_index;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult res = vkCreateCommandPool(__vk_device, &cmdPoolInfo, nullptr, &__vk_commnad_pool);
	if (res != VK_SUCCESS)
	{
		Log::error(L"create command pool failed..");
	}
	return res;
}

VkResult Application::_init_command_buffer() 
{
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = nullptr;
	cmd.commandPool = __vk_commnad_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers(__vk_device, &cmd, &__vk_command_buffer);
	if (res != VK_SUCCESS)
	{
		Log::error(L"create command buffer failed..");
	}
	return res;
}

VkResult Application::_execute_begin_command_buffer()
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;
	cmdBufInfo.flags = 0;
	cmdBufInfo.pInheritanceInfo = nullptr;

	VkResult res = vkBeginCommandBuffer(__vk_command_buffer, &cmdBufInfo);
	if (res != VK_SUCCESS)
	{
		Log::error(L"begin command buffer failed..");
	}
	return res;
}

void Application::_init_device_queue()
{
	vkGetDeviceQueue(__vk_device, __graphics_queue_family_index, 0, &__vk_graphics_queue);
	if (__graphics_queue_family_index == __present_queue_family_index)
	{
		__vk_present_queue = __vk_graphics_queue;
	}
	else
	{
		vkGetDeviceQueue(__vk_device, __present_queue_family_index, 0, &__vk_present_queue);
	}
}

VkResult Application::_init_swap_chain(VkImageUsageFlags usageFlags)
{
	VkResult res;
	VkSurfaceCapabilitiesKHR surfCapabilities;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(__vk_gpus[0], __vk_surface, &surfCapabilities);
	if (res != VK_SUCCESS)
	{
		Log::error(L"vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed..");
		return res;
	}

	uint32_t presentModeCount = 0;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(__vk_gpus[0], __vk_surface, &presentModeCount, nullptr);
	if (res != VK_SUCCESS)
	{
		Log::error(L"vkGetPhysicalDeviceSurfacePresentModesKHR failed..");
		return res;
	}

	VkPresentModeKHR *presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(__vk_gpus[0], __vk_surface, &presentModeCount, presentModes);
	
	VkExtent2D swapchainExtent;
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchainExtent.width  = __client_width;
		swapchainExtent.height = __client_height;
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) 
		{
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	}
	else
	{
		swapchainExtent = surfCapabilities.currentExtent;
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = { };
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.surface = __vk_surface;
	swapchainCreateInfo.minImageCount = desiredNumberOfSwapChainImages;
	swapchainCreateInfo.imageFormat = __surface_format;
	swapchainCreateInfo.imageExtent.width = swapchainExtent.width;
	swapchainCreateInfo.imageExtent.height = swapchainExtent.height;
	swapchainCreateInfo.preTransform = preTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	
	swapchainCreateInfo.clipped = true;

	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageUsage = usageFlags;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	uint32_t queueFamilyIndices[2] = {
		(uint32_t)__graphics_queue_family_index,
		(uint32_t)__present_queue_family_index
	};

	if (__graphics_queue_family_index != __present_queue_family_index)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	res = vkCreateSwapchainKHR(__vk_device, &swapchainCreateInfo, nullptr, &__vk_swapchain);
	if (res)
	{
		Log::error(L"vkCreateSwapchainKHR failed..");
		return res;
	}
	res = vkGetSwapchainImagesKHR(__vk_device, __vk_swapchain, &__swapchain_image_count, nullptr);
	if (res)
	{
		Log::error(L"vkGetSwapchainImagesKHR failed..");
		return res;
	}
	Log::info(L"swapchain image count: %u", __swapchain_image_count);

	VkImage* swapchainImages = (VkImage*)malloc(__swapchain_image_count * sizeof(VkImage));
	res = vkGetSwapchainImagesKHR(__vk_device, __vk_swapchain, &__swapchain_image_count, swapchainImages);

	for (uint32_t i = 0; i < __swapchain_image_count; i++)
	{
		SwapChainBuffer scBuffer;
		VkImageViewCreateInfo colorImageViewCreateInfo = {};
		colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageViewCreateInfo.pNext = nullptr;
		colorImageViewCreateInfo.format = __surface_format;
		colorImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		colorImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		colorImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		colorImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		colorImageViewCreateInfo.subresourceRange.levelCount = 1;
		colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		colorImageViewCreateInfo.subresourceRange.layerCount = 1;
		colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageViewCreateInfo.flags = 0;

		scBuffer.image = swapchainImages[i];
		colorImageViewCreateInfo.image = scBuffer.image;

		res = vkCreateImageView(__vk_device, &colorImageViewCreateInfo, nullptr, &scBuffer.view);
		if (res)
		{
			Log::error(L"vkCreateImageView failed..");
			return res;
		}
		__vk_swapchain_buffers.push_back(scBuffer);
	}
	free(swapchainImages);

	__current_swapchain_buffer = 0;

	if (presentModes != nullptr)
	{
		free(presentModes);
	}
	return res;
}