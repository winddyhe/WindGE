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
	for (uint32_t i = 0; i < __swapchain_image_count; i++) 
	{
		vkDestroyFramebuffer(__vk_device, __vk_framebuffers[i], NULL);
	}
	free(__vk_framebuffers);
	__vk_framebuffers = nullptr;

	vkDestroyShaderModule(__vk_device, __vk_pipeline_shaderstages[0].module, nullptr);
	vkDestroyShaderModule(__vk_device, __vk_pipeline_shaderstages[1].module, nullptr);

	vkDestroyRenderPass(__vk_device, __vk_render_pass, nullptr);

	for (int i = 0; i < 1; i++)
	{
		vkDestroyDescriptorSetLayout(__vk_device, __vk_desc_layouts[i], nullptr);
	}
	vkDestroyPipelineLayout(__vk_device, __vk_pipeline_layout, nullptr);

	vkDestroyBuffer(__vk_device, __uniform_mvp.buf, nullptr);
	vkFreeMemory(__vk_device, __uniform_mvp.mem, nullptr);

	vkDestroyImageView(__vk_device, __vk_depth.view, nullptr);
	vkDestroyImage(__vk_device, __vk_depth.image, nullptr);
	vkFreeMemory(__vk_device, __vk_depth.mem, nullptr);

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

	if (!_init_depth_buffer())			  return false;
	if (!_init_uniform_buffer())		  return false;

	if (!_init_descriptor_pipeline_layouts(false)) return false;

	if (!_init_renderpass(true))		  return false;

	if (!_init_shaders())				  return false;

	if (!_init_frame_buffers(true))		  return false;

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
	instInfo.enabledExtensionCount = (unsigned int)__inst_extension_names.size();
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

	vkGetPhysicalDeviceMemoryProperties(__vk_gpus[0], &__vk_memory_props);
	vkGetPhysicalDeviceProperties(__vk_gpus[0], &__vk_gpu_props);

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
	deviceInfo.enabledExtensionCount = (unsigned int)__device_extension_names.size();
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

VkResult Application::_execute_end_command_buffer() 
{
	return vkEndCommandBuffer(__vk_command_buffer);	
}

VkResult Application::_execute_queue_command_buffer() 
{
	VkResult res;
	const VkCommandBuffer cmdBufs[] = { __vk_command_buffer };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	res = vkCreateFence(__vk_device, &fenceInfo, NULL, &drawFence);

	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo[1] = {};
	submitInfo[0].pNext = NULL;
	submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo[0].waitSemaphoreCount = 0;
	submitInfo[0].pWaitSemaphores = NULL;
	submitInfo[0].pWaitDstStageMask = &pipeStageFlags;
	submitInfo[0].commandBufferCount = 1;
	submitInfo[0].pCommandBuffers = cmdBufs;
	submitInfo[0].signalSemaphoreCount = 0;
	submitInfo[0].pSignalSemaphores = NULL;

	res = vkQueueSubmit(__vk_graphics_queue, 1, submitInfo, drawFence);
	
	do {
		res = vkWaitForFences(__vk_device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
	} while (res == VK_TIMEOUT);
	
	vkDestroyFence(__vk_device, drawFence, NULL);

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

bool Application::_init_depth_buffer()
{
	VkResult res;
	bool pass = false;
	VkImageCreateInfo imageInfo = {};

	if (__vk_depth.format == VK_FORMAT_UNDEFINED)
		__vk_depth.format = VK_FORMAT_D16_UNORM;

	const VkFormat depthFormat = __vk_depth.format;

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(__vk_gpus[0], depthFormat, &props);
	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else
	{
		Log::error(L"Depth format %d unsupported.", (int)depthFormat);
		return false;
	}

	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthFormat;
	imageInfo.extent.width = __client_width;
	imageInfo.extent.height = __client_height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.flags = 0;

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = nullptr;
	memAllocInfo.allocationSize = 0;
	memAllocInfo.memoryTypeIndex = 0;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.image = VK_NULL_HANDLE;
	viewInfo.format = depthFormat;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.flags = 0;

	if (depthFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
		depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkMemoryRequirements memReqs;
	res = vkCreateImage(__vk_device, &imageInfo, nullptr, &__vk_depth.image);
	if (res) return false;

	vkGetImageMemoryRequirements(__vk_device, __vk_depth.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;

	pass = _memory_type_from_properties(memReqs.memoryTypeBits, 0, &memAllocInfo.memoryTypeIndex);
	if (!pass) return false;

	res = vkAllocateMemory(__vk_device, &memAllocInfo, nullptr, &__vk_depth.mem);

	res = vkBindImageMemory(__vk_device, __vk_depth.image, __vk_depth.mem, 0);
	if (res) return false;

	_set_image_layout(__vk_depth.image, viewInfo.subresourceRange.aspectMask, 
					  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	viewInfo.image = __vk_depth.image;
	res = vkCreateImageView(__vk_device, &viewInfo, nullptr, &__vk_depth.view);
	if (res) return false;

	return true;
}

bool Application::_memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	for (uint32_t i = 0; i < __vk_memory_props.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((__vk_memory_props.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

bool Application::_set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout)
{
	if (__vk_command_buffer != VK_NULL_HANDLE) return false;
	if (__vk_graphics_queue != VK_NULL_HANDLE) return false;

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = 0;
	imageMemoryBarrier.oldLayout = old_image_layout;
	imageMemoryBarrier.newLayout = new_image_layout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) 
	{
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) 
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) 
	{
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
	{
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(__vk_command_buffer, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);

	return true;
}

bool Application::_init_uniform_buffer()
{
	VkResult res;
	bool pass;

	float fov = glm::radians(45.0f);
	if (__client_width > __client_height)
	{
		fov *= static_cast<float>(__client_height) / static_cast<float>(__client_width);
	}
	
	float aspect = static_cast<float>(__client_width) / static_cast<float>(__client_height);
	
	__proj_mat  = glm::perspective(fov, aspect, 0.1f, 100.0f);
	__view_mat  = glm::lookAt(glm::vec3(-5, 3, -10), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
	__model_mat = glm::mat4(1.0f);

	__clip_mat  = glm::mat4(1.0f,  0.0f, 0.0f, 0.0f,
							0.0f, -1.0f, 0.0f, 0.0f,
							0.0f,  0.0f, 0.5f, 0.0f,
							0.0f,  0.0f, 0.5f, 1.0f);

	__mvp_mat   = __clip_mat * __proj_mat * __view_mat * __model_mat;

	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = nullptr;
	bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufInfo.size = sizeof(__mvp_mat);
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = nullptr;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;
	
	res = vkCreateBuffer(__vk_device, &bufInfo, nullptr, &__uniform_mvp.buf);
	if (res) return false;

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(__vk_device, __uniform_mvp.buf, &memReqs);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memReqs.size;

	pass = _memory_type_from_properties(
		memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);

	if (!pass) 
	{
		Log::error(L"No mappable, coherent memory..");
		return false;
	}

	res = vkAllocateMemory(__vk_device, &allocInfo, nullptr, &__uniform_mvp.mem);
	if (res) return false;

	uint8_t *pData;
	res = vkMapMemory(__vk_device, __uniform_mvp.mem, 0, memReqs.size, 0, (void**)&pData);
	if (res) return false;

	memcpy(pData, &__mvp_mat, sizeof(__mvp_mat));

	vkUnmapMemory(__vk_device, __uniform_mvp.mem);

	res = vkBindBufferMemory(__vk_device, __uniform_mvp.buf, __uniform_mvp.mem, 0);
	if (res) return false;

	__uniform_mvp.bufferInfo.buffer = __uniform_mvp.buf;
	__uniform_mvp.bufferInfo.offset = 0;
	__uniform_mvp.bufferInfo.range = sizeof(__mvp_mat);

	return true;
}

bool Application::_init_descriptor_pipeline_layouts(bool useTexture)
{
	VkResult res;

	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = nullptr;

	if (useTexture)
	{
		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = useTexture ? 2 : 1;
	descriptorLayoutInfo.pBindings = layoutBindings;

	__vk_desc_layouts.resize(1);
	res = vkCreateDescriptorSetLayout(__vk_device, &descriptorLayoutInfo, nullptr, __vk_desc_layouts.data());
	if (res)
	{
		Log::error(L"Create desc layout failed...");
		return false;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = __vk_desc_layouts.data();

	res = vkCreatePipelineLayout(__vk_device, &pipelineLayoutCreateInfo, nullptr, &__vk_pipeline_layout);
	if (res) return false;

	return true;
}

bool Application::_init_renderpass(bool includeDepth, bool clear/* = true*/, VkImageLayout finalLayout/* = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR*/)
{
	VkResult res;

	VkAttachmentDescription attachments[2];
	attachments[0].format = __surface_format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = finalLayout;
	attachments[0].flags = 0;

	if (includeDepth)
	{
		attachments[1].format = __vk_depth.format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
	}

	VkAttachmentReference colorRef = {};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef = {};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = includeDepth ? &depthRef : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = includeDepth ? 2 : 1;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	res = vkCreateRenderPass(__vk_device, &renderPassInfo, nullptr, &__vk_render_pass);
	if (res)
	{
		Log::error(L"Create render pass failed...");
		return false;
	}

	return true;
}

EShLanguage Application::__find_language(const VkShaderStageFlagBits shaderType) 
{
	switch (shaderType) 
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

	case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}

void Application::__init_shader_resources(TBuiltInResource &resources)
{
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = 32;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = 8;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.limits.nonInductiveForLoops = 1;
	resources.limits.whileLoops = 1;
	resources.limits.doWhileLoops = 1;
	resources.limits.generalUniformIndexing = 1;
	resources.limits.generalAttributeMatrixVectorIndexing = 1;
	resources.limits.generalVaryingIndexing = 1;
	resources.limits.generalSamplerIndexing = 1;
	resources.limits.generalVariableIndexing = 1;
	resources.limits.generalConstantMatrixVectorIndexing = 1;
}

bool Application::__glsl_to_spv(const VkShaderStageFlagBits shaderType, const char* pShader, std::vector<unsigned int> &spirv)
{
	EShLanguage stage = __find_language(shaderType);
	glslang::TShader shader(stage);
	glslang::TProgram program;

	const char* shaderString[1];
	TBuiltInResource shaderResource;
	__init_shader_resources(shaderResource);

	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	shaderString[0] = pShader;
	shader.setStrings(shaderString, 1);

	if (!shader.parse(&shaderResource, 100, false, messages))
	{
		Log::error(shader.getInfoLog());
		Log::error(shader.getInfoDebugLog());
		return false;
	}

	program.addShader(&shader);
	if (!program.link(messages))
	{
		Log::error(shader.getInfoLog());
		Log::error(shader.getInfoDebugLog());
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
	return true;
}

bool Application::_init_shaders()
{
	static const char* vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform bufferVals\n"
		"{\n"
		"	mat4 mvp;\n"
		"} myBufferVals;\n"
		"layout (location = 0) in  vec4 pos;\n"
		"layout (location = 1) in  vec4 inColor;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"	outColor = inColor;\n"
		"	gl_Position = myBufferVals.mvp * pos;\n"
		"}\n";

	static const char* fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (location = 0) in  vec4 color;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"	outColor = color;\n"
		"}\n";

	std::vector<unsigned int> vertexSpv;
	__vk_pipeline_shaderstages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	__vk_pipeline_shaderstages[0].pNext = nullptr;
	__vk_pipeline_shaderstages[0].pSpecializationInfo = nullptr;
	__vk_pipeline_shaderstages[0].flags = 0;
	__vk_pipeline_shaderstages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	__vk_pipeline_shaderstages[0].pName = "main";

	glslang::InitializeProcess();

	if (!this->__glsl_to_spv(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vertexSpv))
	{
		Log::error(L"Vertex shader compile error!");
		return false;
	}

	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = vertexSpv.size() * sizeof(unsigned int);
	shaderModuleCreateInfo.pCode = vertexSpv.data();
	if (vkCreateShaderModule(__vk_device, &shaderModuleCreateInfo, nullptr, &__vk_pipeline_shaderstages[0].module))
	{
		Log::error(L"Create vertex shader error!");
		return false;
	}

	std::vector<unsigned int> fragSpv;
	__vk_pipeline_shaderstages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	__vk_pipeline_shaderstages[1].pNext = nullptr;
	__vk_pipeline_shaderstages[1].pSpecializationInfo = nullptr;
	__vk_pipeline_shaderstages[1].flags = 0;
	__vk_pipeline_shaderstages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	__vk_pipeline_shaderstages[1].pName = "main";

	if (!this->__glsl_to_spv(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, fragSpv))
	{
		Log::error(L"Frag shader compile error!");
		return false;
	}

	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = fragSpv.size() * sizeof(unsigned int);
	shaderModuleCreateInfo.pCode = fragSpv.data();
	if (vkCreateShaderModule(__vk_device, &shaderModuleCreateInfo, nullptr, &__vk_pipeline_shaderstages[1].module))
	{
		Log::error(L"Create frag shader error!");
		return false;
	}

	glslang::FinalizeProcess();

	Log::info("Init shaders success");

	return true;
}

bool Application::_init_frame_buffers(bool includeDepth) 
{
	VkImageView attachments[2];
	attachments[1] = __vk_depth.view;

	VkFramebufferCreateInfo frameBufferCreateInfo;
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.renderPass = __vk_render_pass;
	frameBufferCreateInfo.attachmentCount = includeDepth ? 2 : 1;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = __client_width;
	frameBufferCreateInfo.height = __client_height;
	frameBufferCreateInfo.layers = 1;

	uint32_t i = 0;
	__vk_framebuffers = (VkFramebuffer*)malloc(__swapchain_image_count * sizeof(VkFramebuffer));

	for (i = 0; i < __swapchain_image_count; i++)
	{
		attachments[0] = __vk_swapchain_buffers[i].view;
		if (vkCreateFramebuffer(__vk_device, &frameBufferCreateInfo, nullptr, &__vk_framebuffers[i]))
		{
			Log::error("Create framebuffer error!");
			return false;
		}
	}

	if (_execute_end_command_buffer())
	{
		Log::error("Execute end command buffer error!");
		return false;
	}

	if (_execute_queue_command_buffer())
	{
		Log::error("Execute queue command buffer error!");
		return false;
	}
	Log::info("Init framebuffers success");
	return true;
}