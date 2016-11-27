#pragma once
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <memory>
#include <vector>

#include "Config.h"
#include "vulkan\vulkan.hpp"
#include "vulkan\vk_sdk_platform.h"

namespace WindGE
{
	class WIND_CORE_API Application
	{
		typedef struct 
		{
			VkLayerProperties						props;
			std::vector<VkExtensionProperties>		extensionProps;
		} LayerProperties;

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

	protected:
		VkInstance								__vk_inst;
		std::vector<VkPhysicalDevice>			__vk_gpus;
		std::vector<VkQueueFamilyProperties>	__vk_queue_family_props;
		VkDevice								__vk_device;
		VkCommandPool							__vk_cmd_pool;
		VkCommandBuffer							__vk_cmd_buffer;
		VkSurfaceKHR							__vk_surface;

		int										__client_width;
		int										__client_height;

		std::vector<LayerProperties>			__layer_props_vec;
		std::vector<const char*>				__inst_extension_names;
	};
}

#endif // !__APPLICATION_H__

