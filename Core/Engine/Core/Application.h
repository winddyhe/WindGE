#pragma once
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <memory>
#include <vector>

#include "Config.h"
#include "vulkan\vulkan.hpp"
#include "vulkan\vk_sdk_platform.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "Log.h"

namespace WindGE
{
	class WIND_CORE_API Application
	{
		typedef struct 
		{
			VkLayerProperties						props;
			std::vector<VkExtensionProperties>		extensionProps;
		} LayerProperties;

		typedef struct
		{
			VkImage									image;
			VkImageView								view;
		} SwapChainBuffer;

		typedef struct
		{
			VkFormat								format;
			VkImage									image;
			VkDeviceMemory							mem;
			VkImageView								view;
		} Depth;

		typedef struct
		{
			VkBuffer								buf;
			VkDeviceMemory							mem;
			VkDescriptorBufferInfo					bufferInfo;
		} UniformData;

	public:
		Application();
		virtual ~Application();

	public:
		virtual bool init(HINSTANCE inst, HWND hwnd, int width, int height);
		virtual void update(float deltaTime);
		virtual void draw();
		virtual void resize(int w, int h);

		inline VkDevice* device() { return &__vk_device; }

	protected:
		VkResult _init_global_extension_properties(LayerProperties& layerProps);
		VkResult _init_global_layer_properties();
		VkResult _init_instance_extension_names();
		VkResult _init_device_extension_names();

		VkResult _init_instance();
		bool	 _init_enumerate_device();
		bool	 _init_surface_khr();
		VkResult _init_device();

		VkResult _init_command_pool();
		VkResult _init_command_buffer();
		VkResult _execute_begin_command_buffer();

		void	 _init_device_queue();

		VkResult _init_swap_chain(VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		
		bool	 _init_depth_buffer();
		bool	 _init_uniform_buffer();
		bool     _init_descriptor_pipeline_layouts(bool useTexture);
		bool	 _init_renderpass(bool includeDepth, bool clear = true, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		VkResult _init_shaders();
		VkResult _init_frame_buffers();

		bool	 _memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
		bool     _set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

	protected:
		VkInstance								__vk_inst;
		std::vector<VkPhysicalDevice>			__vk_gpus;
		std::vector<VkQueueFamilyProperties>	__vk_queue_family_props;
		VkPhysicalDeviceMemoryProperties		__vk_memory_props;
		VkPhysicalDeviceProperties				__vk_gpu_props;
		VkDevice								__vk_device;
		VkSurfaceKHR							__vk_surface;
		VkCommandPool							__vk_commnad_pool;
		VkCommandBuffer							__vk_command_buffer;
		VkQueue									__vk_graphics_queue;
		VkQueue									__vk_present_queue;
		VkSwapchainKHR							__vk_swapchain;
		std::vector<SwapChainBuffer>			__vk_swapchain_buffers;
		Depth									__vk_depth;
		std::vector<VkDescriptorSetLayout>		__vk_desc_layouts;
		VkPipelineLayout						__vk_pipeline_layout;
		VkRenderPass							__vk_render_pass;

		int										__client_width;
		int										__client_height;
		HINSTANCE								__window_instance;
		HWND									__window_hwnd;
		std::string								__app_short_name;

		std::vector<LayerProperties>			__layer_props_vec;
		std::vector<const char*>				__inst_extension_names;
		std::vector<const char*>				__device_extension_names;
		uint32_t								__queue_family_count;
		uint32_t								__queue_family_index;
		uint32_t								__graphics_queue_family_index;
		uint32_t								__present_queue_family_index;
		uint32_t								__current_swapchain_buffer;

		uint32_t								__swapchain_image_count;
		
		VkFormat								__surface_format;

		glm::mat4								__proj_mat;
		glm::mat4								__view_mat;
		glm::mat4								__model_mat;
		glm::mat4								__clip_mat;
		glm::mat4								__mvp_mat;

		UniformData								__uniform_mvp;
	};
}

#endif // !__APPLICATION_H__

