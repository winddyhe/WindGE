#include "Application.h"

#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace WindGE;

Application::Application() :
	__render_device(RenderDevice::instance()),
	__vk_framebuffers(nullptr),
	__client_width(0),
	__client_height(0),
	__swapchain_image_count(0),
	__current_swapchain_buffer(0)
{
}

Application::~Application()
{
	vkDestroyPipeline(__render_device.device(), __vk_pipeline, nullptr);
	vkDestroyPipelineCache(__render_device.device(), __vk_pipeline_cache, nullptr);
	vkDestroyDescriptorPool(__render_device.device(), __vk_descriptor_pool, nullptr);
	vkDestroyBuffer(__render_device.device(), __vk_vertex_buffer.buf, nullptr);
	vkFreeMemory(__render_device.device(), __vk_vertex_buffer.mem, nullptr);

	for (uint32_t i = 0; i < __swapchain_image_count; i++) 
	{
		vkDestroyFramebuffer(__render_device.device(), __vk_framebuffers[i], NULL);
	}
	free(__vk_framebuffers);
	__vk_framebuffers = nullptr;

	vkDestroyShaderModule(__render_device.device(), __vk_pipeline_shaderstages[0].module, nullptr);
	vkDestroyShaderModule(__render_device.device(), __vk_pipeline_shaderstages[1].module, nullptr);

	vkDestroyRenderPass(__render_device.device(), __vk_render_pass, nullptr);

	for (int i = 0; i < 1; i++)
	{
		vkDestroyDescriptorSetLayout(__render_device.device(), __vk_desc_layouts[i], nullptr);
	}
	vkDestroyPipelineLayout(__render_device.device(), __vk_pipeline_layout, nullptr);

	vkDestroyBuffer(__render_device.device(), __uniform_mvp.buf, nullptr);
	vkFreeMemory(__render_device.device(), __uniform_mvp.mem, nullptr);

	vkDestroyImageView(__render_device.device(), __vk_depth.view, nullptr);
	vkDestroyImage(__render_device.device(), __vk_depth.image, nullptr);
	vkFreeMemory(__render_device.device(), __vk_depth.mem, nullptr);

	if (__render_device.device())
	{
		for (uint32_t i = 0; i < __swapchain_image_count; i++) 
		{
			vkDestroyImageView(__render_device.device(), __vk_swapchain_buffers[i].view, NULL);
		}
		vkDestroySwapchainKHR(__render_device.device(), __vk_swapchain, NULL);

		VkCommandBuffer cmdBufs[1] = { __vk_command_buffer };
		vkFreeCommandBuffers(__render_device.device(), __vk_commnad_pool, 1, cmdBufs);
		vkDestroyCommandPool(__render_device.device(), __vk_commnad_pool, nullptr);
	}
	__render_device.release();
}

bool Application::init(HINSTANCE inst, HWND hwnd, int width, int height)
{
	__client_width    = width;
	__client_height   = height;

	if (!__render_device.init(inst, hwnd))return false;

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
	if (!_init_vertex_buffer(g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data), sizeof(g_vb_solid_face_colors_Data[0]), false)) return false;
	if (!_init_descriptor_pool(false))	  return false;
	if (!_init_descriptor_set(false))	  return false;
	if (!_init_pipeline_cache())		  return false;
	if (!_init_pipeline(true))			  return false;

	if (!_draw_cube())					  return false;

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

VkResult Application::_init_command_pool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = nullptr;
	cmdPoolInfo.queueFamilyIndex = __render_device.graphics_queue_family_index();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult res = vkCreateCommandPool(__render_device.device(), &cmdPoolInfo, nullptr, &__vk_commnad_pool);
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

	VkResult res = vkAllocateCommandBuffers(__render_device.device(), &cmd, &__vk_command_buffer);
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
	res = vkCreateFence(__render_device.device(), &fenceInfo, NULL, &drawFence);

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
		res = vkWaitForFences(__render_device.device(), 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
	} while (res == VK_TIMEOUT);
	
	vkDestroyFence(__render_device.device(), drawFence, NULL);

	return res;
}

void Application::_init_device_queue()
{
	vkGetDeviceQueue(__render_device.device(), __render_device.graphics_queue_family_index(), 0, &__vk_graphics_queue);
	if (__render_device.graphics_queue_family_index() == __render_device.present_queue_family_index())
	{
		__vk_present_queue = __vk_graphics_queue;
	}
	else
	{
		vkGetDeviceQueue(__render_device.device(), __render_device.present_queue_family_index(), 0, &__vk_present_queue);
	}
}

VkResult Application::_init_swap_chain(VkImageUsageFlags usageFlags)
{
	VkResult res;
	VkSurfaceCapabilitiesKHR surfCapabilities;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(__render_device.gpus()[0], __render_device.surface(), &surfCapabilities);
	if (res != VK_SUCCESS)
	{
		Log::error(L"vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed..");
		return res;
	}

	uint32_t presentModeCount = 0;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(__render_device.gpus()[0], __render_device.surface(), &presentModeCount, nullptr);
	if (res != VK_SUCCESS)
	{
		Log::error(L"vkGetPhysicalDeviceSurfacePresentModesKHR failed..");
		return res;
	}

	VkPresentModeKHR *presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(__render_device.gpus()[0], __render_device.surface(), &presentModeCount, presentModes);
	
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
	swapchainCreateInfo.surface = __render_device.surface();
	swapchainCreateInfo.minImageCount = desiredNumberOfSwapChainImages;
	swapchainCreateInfo.imageFormat = __render_device.surface_format();
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
		(uint32_t)__render_device.graphics_queue_family_index(),
		(uint32_t)__render_device.present_queue_family_index()
	};

	if (__render_device.graphics_queue_family_index() != __render_device.present_queue_family_index())
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	res = vkCreateSwapchainKHR(__render_device.device(), &swapchainCreateInfo, nullptr, &__vk_swapchain);
	if (res)
	{
		Log::error(L"vkCreateSwapchainKHR failed..");
		return res;
	}
	res = vkGetSwapchainImagesKHR(__render_device.device(), __vk_swapchain, &__swapchain_image_count, nullptr);
	if (res)
	{
		Log::error(L"vkGetSwapchainImagesKHR failed..");
		return res;
	}
	Log::info(L"swapchain image count: %u", __swapchain_image_count);

	VkImage* swapchainImages = (VkImage*)malloc(__swapchain_image_count * sizeof(VkImage));
	res = vkGetSwapchainImagesKHR(__render_device.device(), __vk_swapchain, &__swapchain_image_count, swapchainImages);

	for (uint32_t i = 0; i < __swapchain_image_count; i++)
	{
		SwapChainBuffer scBuffer;
		VkImageViewCreateInfo colorImageViewCreateInfo = {};
		colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageViewCreateInfo.pNext = nullptr;
		colorImageViewCreateInfo.format = __render_device.surface_format();
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

		res = vkCreateImageView(__render_device.device(), &colorImageViewCreateInfo, nullptr, &scBuffer.view);
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
	vkGetPhysicalDeviceFormatProperties(__render_device.gpus()[0], depthFormat, &props);
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
	res = vkCreateImage(__render_device.device(), &imageInfo, nullptr, &__vk_depth.image);
	if (res) return false;

	vkGetImageMemoryRequirements(__render_device.device(), __vk_depth.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;

	pass = _memory_type_from_properties(memReqs.memoryTypeBits, 0, &memAllocInfo.memoryTypeIndex);
	if (!pass) return false;

	res = vkAllocateMemory(__render_device.device(), &memAllocInfo, nullptr, &__vk_depth.mem);

	res = vkBindImageMemory(__render_device.device(), __vk_depth.image, __vk_depth.mem, 0);
	if (res) return false;

	_set_image_layout(__vk_depth.image, viewInfo.subresourceRange.aspectMask, 
					  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	viewInfo.image = __vk_depth.image;
	res = vkCreateImageView(__render_device.device(), &viewInfo, nullptr, &__vk_depth.view);
	if (res) return false;

	return true;
}

bool Application::_memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	for (uint32_t i = 0; i < __render_device.memory_props().memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((__render_device.memory_props().memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
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
	
	res = vkCreateBuffer(__render_device.device(), &bufInfo, nullptr, &__uniform_mvp.buf);
	if (res) return false;

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(__render_device.device(), __uniform_mvp.buf, &memReqs);

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

	res = vkAllocateMemory(__render_device.device(), &allocInfo, nullptr, &__uniform_mvp.mem);
	if (res) return false;

	uint8_t *pData;
	res = vkMapMemory(__render_device.device(), __uniform_mvp.mem, 0, memReqs.size, 0, (void**)&pData);
	if (res) return false;

	memcpy(pData, &__mvp_mat, sizeof(__mvp_mat));

	vkUnmapMemory(__render_device.device(), __uniform_mvp.mem);

	res = vkBindBufferMemory(__render_device.device(), __uniform_mvp.buf, __uniform_mvp.mem, 0);
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
	res = vkCreateDescriptorSetLayout(__render_device.device(), &descriptorLayoutInfo, nullptr, __vk_desc_layouts.data());
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

	res = vkCreatePipelineLayout(__render_device.device(), &pipelineLayoutCreateInfo, nullptr, &__vk_pipeline_layout);
	if (res) return false;

	return true;
}

bool Application::_init_renderpass(bool includeDepth, bool clear/* = true*/, VkImageLayout finalLayout/* = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR*/)
{
	VkResult res;

	VkAttachmentDescription attachments[2];
	attachments[0].format = __render_device.surface_format();
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

	res = vkCreateRenderPass(__render_device.device(), &renderPassInfo, nullptr, &__vk_render_pass);
	if (res)
	{
		Log::error(L"Create render pass failed...");
		return false;
	}

	return true;
}

EShLanguage Application::_find_language(const VkShaderStageFlagBits shaderType) 
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

void Application::_init_shader_resources(TBuiltInResource &resources)
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

bool Application::_glsl_to_spv(const VkShaderStageFlagBits shaderType, const char* pShader, std::vector<unsigned int> &spirv)
{
	EShLanguage stage = _find_language(shaderType);
	glslang::TShader shader(stage);
	glslang::TProgram program;

	const char* shaderString[1];
	TBuiltInResource shaderResource;
	_init_shader_resources(shaderResource);

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

	if (!this->_glsl_to_spv(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vertexSpv))
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
	if (vkCreateShaderModule(__render_device.device(), &shaderModuleCreateInfo, nullptr, &__vk_pipeline_shaderstages[0].module))
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

	if (!this->_glsl_to_spv(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, fragSpv))
	{
		Log::error(L"Frag shader compile error!");
		return false;
	}

	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = fragSpv.size() * sizeof(unsigned int);
	shaderModuleCreateInfo.pCode = fragSpv.data();
	if (vkCreateShaderModule(__render_device.device(), &shaderModuleCreateInfo, nullptr, &__vk_pipeline_shaderstages[1].module))
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
		if (vkCreateFramebuffer(__render_device.device(), &frameBufferCreateInfo, nullptr, &__vk_framebuffers[i]))
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

bool Application::_init_vertex_buffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture)
{
	VkResult res;
	bool pass;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.size = dataSize;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags = 0;

	res = vkCreateBuffer(__render_device.device(), &bufferCreateInfo, nullptr, &__vk_vertex_buffer.buf);
	if (res != VK_SUCCESS)
	{
		Log::error("Create vertex buffer error!");
		return false;
	}

	VkMemoryRequirements memRequires;
	vkGetBufferMemoryRequirements(__render_device.device(), __vk_vertex_buffer.buf, &memRequires);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = nullptr;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = memRequires.size;

	pass = _memory_type_from_properties(memRequires.memoryTypeBits, 
										VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
										&memAllocInfo.memoryTypeIndex);
	if (!pass)
	{
		Log::error("Create vertex buffer info error, No mappable, coherent memory!");
		return false;
	}
	
	res = vkAllocateMemory(__render_device.device(), &memAllocInfo, nullptr, &__vk_vertex_buffer.mem);
	if (res != VK_SUCCESS)
	{
		Log::error("Create vertex buffer info error, allocate memory failed!");
		return false;
	}
	__vk_vertex_buffer.buffer_info.range = memRequires.size;
	__vk_vertex_buffer.buffer_info.offset = 0;

	uint8_t *pData;
	res = vkMapMemory(__render_device.device(), __vk_vertex_buffer.mem, 0, memRequires.size, 0, (void**)&pData);
	if (res != VK_SUCCESS)
	{
		Log::error("Create vertex buffer info error, map memory failed!");
		return false;
	}
	memcpy(pData, vertexData, dataSize);
	vkUnmapMemory(__render_device.device(), __vk_vertex_buffer.mem);

	res = vkBindBufferMemory(__render_device.device(), __vk_vertex_buffer.buf, __vk_vertex_buffer.mem, 0);
	if (res != VK_SUCCESS)
	{
		Log::error("Create vertex buffer info error, bind buffer memory failed!");
		return false;
	}

	__vk_vi_binding.binding = 0;
	__vk_vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	__vk_vi_binding.stride = dataStride;

	__vk_vi_attribs[0].binding = 0;
	__vk_vi_attribs[0].location = 0;
	__vk_vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	__vk_vi_attribs[0].offset = 0;

	__vk_vi_attribs[1].binding = 0;
	__vk_vi_attribs[1].location = 1;
	__vk_vi_attribs[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
	__vk_vi_attribs[1].offset = 16;

	Log::info("Init vertexbuffer success");
	return true;
}

bool Application::_init_descriptor_pool(bool useTexture)
{
	VkResult res;
	
	VkDescriptorPoolSize typeCount[2];
	typeCount[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCount[0].descriptorCount = 1;
	if (useTexture)
	{
		typeCount[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		typeCount[1].descriptorCount = 1;
	}
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = useTexture ? 2 : 1;
	descriptorPoolCreateInfo.pPoolSizes = typeCount;

	res = vkCreateDescriptorPool(__render_device.device(), &descriptorPoolCreateInfo, nullptr, &__vk_descriptor_pool);
	if (res != VK_SUCCESS)
	{
		Log::error("Create descriptor pool error.");
		return false;
	}

	return true;
}

bool Application::_init_descriptor_set(bool useTexture)
{
	VkResult res;

	VkDescriptorSetAllocateInfo descriptorAllocInfo = {};
	descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorAllocInfo.pNext = nullptr;
	descriptorAllocInfo.descriptorPool = __vk_descriptor_pool;
	descriptorAllocInfo.descriptorSetCount = NUM_DESCRIPTOR_SETS;
	descriptorAllocInfo.pSetLayouts = __vk_desc_layouts.data();

	__vk_descriptor_sets.resize(NUM_DESCRIPTOR_SETS);
	res = vkAllocateDescriptorSets(__render_device.device(), &descriptorAllocInfo, __vk_descriptor_sets.data());
	if (res != VK_SUCCESS)
	{
		Log::error("Create descriptor set error.");
		return false;
	}

	VkWriteDescriptorSet writes[2];
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = nullptr;
	writes[0].dstSet = __vk_descriptor_sets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &__uniform_mvp.bufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	if (useTexture)
	{
		//TODO:
	}

	vkUpdateDescriptorSets(__render_device.device(), useTexture ? 2 : 1, writes, 0, nullptr);
	return true;
}

bool Application::_init_pipeline_cache() 
{
	VkResult res;

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCreateInfo.pNext = nullptr;
	pipelineCacheCreateInfo.initialDataSize = 0;
	pipelineCacheCreateInfo.pInitialData = nullptr;
	pipelineCacheCreateInfo.flags = 0;

	res = vkCreatePipelineCache(__render_device.device(), &pipelineCacheCreateInfo, nullptr, &__vk_pipeline_cache);
	if (res != VK_SUCCESS)
	{
		Log::error("Create pipline cache error.");
		return false;
	}
	return true;
}

bool Application::_init_pipeline(bool includeDepth, bool includeVi/* = true*/)
{
	VkResult res;

	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables;
	dynamicStateCreateInfo.dynamicStateCount = 0;

	VkPipelineVertexInputStateCreateInfo viStateCreateInfo = {};
	viStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (includeVi)
	{
		viStateCreateInfo.pNext = nullptr;
		viStateCreateInfo.flags = 0;
		viStateCreateInfo.vertexBindingDescriptionCount = 1;
		viStateCreateInfo.pVertexBindingDescriptions = &__vk_vi_binding;
		viStateCreateInfo.vertexAttributeDescriptionCount = 2;
		viStateCreateInfo.pVertexAttributeDescriptions = __vk_vi_attribs;
	}

	VkPipelineInputAssemblyStateCreateInfo iaStateCreateInfo = {};
	iaStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaStateCreateInfo.pNext = nullptr;
	iaStateCreateInfo.flags = 0;
	iaStateCreateInfo.primitiveRestartEnable = VK_FALSE;
	iaStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rsStateCreateInfo = {};
	rsStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rsStateCreateInfo.pNext = nullptr;
	rsStateCreateInfo.flags = 0;
	rsStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rsStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rsStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rsStateCreateInfo.depthClampEnable = VK_FALSE;
	rsStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rsStateCreateInfo.depthBiasEnable = VK_FALSE;
	rsStateCreateInfo.depthBiasConstantFactor = 0;
	rsStateCreateInfo.depthBiasClamp = 0;
	rsStateCreateInfo.depthBiasSlopeFactor = 0;
	rsStateCreateInfo.lineWidth = 1.0f;

	VkPipelineColorBlendStateCreateInfo cbStateCreateInfo = {};
	cbStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbStateCreateInfo.flags = 0;
	cbStateCreateInfo.pNext = nullptr;
	VkPipelineColorBlendAttachmentState attState[1];
	attState[0].colorWriteMask = 0xf;
	attState[0].blendEnable = VK_FALSE;
	attState[0].alphaBlendOp = VK_BLEND_OP_ADD;
	attState[0].colorBlendOp = VK_BLEND_OP_ADD;
	attState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	attState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	attState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	attState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	cbStateCreateInfo.attachmentCount = 1;
	cbStateCreateInfo.pAttachments = attState;
	cbStateCreateInfo.logicOpEnable = VK_FALSE;
	cbStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	cbStateCreateInfo.blendConstants[0] = 1.0f;
	cbStateCreateInfo.blendConstants[1] = 1.0f;
	cbStateCreateInfo.blendConstants[2] = 1.0f;
	cbStateCreateInfo.blendConstants[3] = 1.0f;

	VkPipelineViewportStateCreateInfo vpStateCreateInfo = {};
	vpStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpStateCreateInfo.pNext = nullptr;
	vpStateCreateInfo.flags = 0;

	vpStateCreateInfo.viewportCount = NUM_VIEWPORTS;
	dynamicStateEnables[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vpStateCreateInfo.scissorCount = NUM_SCISSORS;
	dynamicStateEnables[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	vpStateCreateInfo.pScissors = nullptr;
	vpStateCreateInfo.pViewports = nullptr;

	VkPipelineDepthStencilStateCreateInfo dsStateCreateInfo;
	dsStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	dsStateCreateInfo.pNext = nullptr;
	dsStateCreateInfo.flags = 0;
	dsStateCreateInfo.depthTestEnable = includeDepth;
	dsStateCreateInfo.depthWriteEnable = includeDepth;
	dsStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	dsStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	dsStateCreateInfo.stencilTestEnable = VK_FALSE;
	dsStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	dsStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	dsStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	dsStateCreateInfo.back.compareMask = 0;
	dsStateCreateInfo.back.reference = 0;
	dsStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	dsStateCreateInfo.back.writeMask = 0;
	dsStateCreateInfo.minDepthBounds = 0;
	dsStateCreateInfo.maxDepthBounds = 0;
	dsStateCreateInfo.stencilTestEnable = VK_FALSE;
	dsStateCreateInfo.front = dsStateCreateInfo.back;

	VkPipelineMultisampleStateCreateInfo msStateCreateInfo;
	msStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msStateCreateInfo.pNext = nullptr;
	msStateCreateInfo.flags = 0;
	msStateCreateInfo.pSampleMask = nullptr;
	msStateCreateInfo.rasterizationSamples = NUM_SAMPLES;
	msStateCreateInfo.sampleShadingEnable = VK_FALSE;
	msStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	msStateCreateInfo.alphaToOneEnable = VK_FALSE;
	msStateCreateInfo.minSampleShading = 0.0;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.layout = __vk_pipeline_layout;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = 0;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.pVertexInputState = &viStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &iaStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rsStateCreateInfo;
	pipelineCreateInfo.pColorBlendState = &cbStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pMultisampleState = &msStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pViewportState = &vpStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &dsStateCreateInfo;
	pipelineCreateInfo.pStages = __vk_pipeline_shaderstages;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.renderPass = __vk_render_pass;
	pipelineCreateInfo.subpass = 0;

	res = vkCreateGraphicsPipelines(__render_device.device(), __vk_pipeline_cache, 1, &pipelineCreateInfo, nullptr, &__vk_pipeline);
	if (res != VK_SUCCESS)
	{
		Log::error("Init pipeline error, create failed.");
		return false;
	}

	Log::info("Init pipeline completed..");
	return true;
}

bool Application::_draw_cube() 
{
	VkResult res;

	VkClearValue clearValues[2];
	clearValues[0].color.float32[0] = 0.2f;
	clearValues[0].color.float32[1] = 0.2f;
	clearValues[0].color.float32[2] = 0.2f;
	clearValues[0].color.float32[3] = 0.2f;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	VkSemaphore imageAcquiredSemaphore;
	VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
	imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	imageAcquiredSemaphoreCreateInfo.pNext = NULL;
	imageAcquiredSemaphoreCreateInfo.flags = 0;

	res = vkCreateSemaphore(__render_device.device(), &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
	if (res != VK_SUCCESS)
	{
		Log::error("vkCreateSemaphore failed.");
		return false;
	}

	res = vkAcquireNextImageKHR(__render_device.device(), __vk_swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &__current_swapchain_buffer);
	if (res != VK_SUCCESS)
	{
		Log::error("vkAcquireNextImageKHR failed.");
		return false;
	}

	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = nullptr;
	rpBeginInfo.renderPass = __vk_render_pass;
	rpBeginInfo.framebuffer = __vk_framebuffers[__current_swapchain_buffer];
	rpBeginInfo.renderArea.offset.x = 0;
	rpBeginInfo.renderArea.offset.y = 0;
	rpBeginInfo.renderArea.extent.width = __client_width;
	rpBeginInfo.renderArea.extent.height = __client_height;
	rpBeginInfo.clearValueCount = 2;
	rpBeginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(__vk_command_buffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(__vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, __vk_pipeline);
	vkCmdBindDescriptorSets(__vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, __vk_pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
							__vk_descriptor_sets.data(), 0, nullptr);

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(__vk_command_buffer, 0, 1, &__vk_vertex_buffer.buf, offsets);

	_init_viewports();
	_init_scissors();

	vkCmdDraw(__vk_command_buffer, 12 * 3, 1, 0, 0);
	vkCmdEndRenderPass(__vk_command_buffer);
	res = vkEndCommandBuffer(__vk_command_buffer);
	const VkCommandBuffer cmd_bufs[] = { __vk_command_buffer };

	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = 0;
	vkCreateFence(__render_device.device(), &fenceInfo, nullptr, &drawFence);

	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo[1] = {};
	submitInfo[0].pNext = nullptr;
	submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo[0].waitSemaphoreCount = 1;
	submitInfo[0].pWaitSemaphores = &imageAcquiredSemaphore;
	submitInfo[0].pWaitDstStageMask = &pipeStageFlags;
	submitInfo[0].commandBufferCount = 1;
	submitInfo[0].pCommandBuffers = cmd_bufs;
	submitInfo[0].signalSemaphoreCount = 0;
	submitInfo[0].pSignalSemaphores = nullptr;

	res = vkQueueSubmit(__vk_graphics_queue, 1, submitInfo, drawFence);
	if (res != VK_SUCCESS)
	{
		Log::error("vkQueueSubmit failed.");
		return false;
	}

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = nullptr;
	present.swapchainCount = 1;
	present.pSwapchains = &__vk_swapchain;
	present.pImageIndices = &__current_swapchain_buffer;
	present.pWaitSemaphores = nullptr;
	present.waitSemaphoreCount = 0;
	present.pResults = nullptr;

	do 
	{
		res = vkWaitForFences(__render_device.device(), 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
	} 
	while (res == VK_TIMEOUT);
	if (res != VK_SUCCESS)
	{
		Log::error("vkWaitForFences failed.");
		return false;
	}

	res = vkQueuePresentKHR(__vk_present_queue, &present);
	if (res != VK_SUCCESS)
	{
		Log::error("vkQueuePresentKHR failed.");
		return false;
	}

	Sleep(1 * 1000);

	vkDestroySemaphore(__render_device.device(), imageAcquiredSemaphore, nullptr);
	vkDestroyFence(__render_device.device(), drawFence, nullptr);
	return true;
}

void Application::_init_viewports() 
{
	__vk_viewport.height = (float)__client_height;
	__vk_viewport.width = (float)__client_width;
	__vk_viewport.minDepth = (float)0.0f;
	__vk_viewport.maxDepth = (float)1.0f;
	__vk_viewport.x = 0;
	__vk_viewport.y = 0;

	vkCmdSetViewport(__vk_command_buffer, 0, NUM_VIEWPORTS, &__vk_viewport);
}

void Application::_init_scissors() 
{
	__vk_scissor.extent.width = __client_width;
	__vk_scissor.extent.height = __client_height;
	__vk_scissor.offset.x = 0;
	__vk_scissor.offset.y = 0;

	vkCmdSetScissor(__vk_command_buffer, 0, NUM_SCISSORS, &__vk_scissor);
}