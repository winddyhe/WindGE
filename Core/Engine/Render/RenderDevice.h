#pragma once
#ifndef __RENDER_DEVICE_H__
#define __RENDER_DEVICE_H__

#include "../Core/Config.h"
#include "../Core/Log.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vk_sdk_platform.h"

namespace WindGE 
{
	class WIND_CORE_API RenderDevice 
	{
		typedef struct
		{
			VkLayerProperties						props;
			std::vector<VkExtensionProperties>		extensionProps;
		} LayerProperties;

	public:
		static RenderDevice& instance() 
		{
			static RenderDevice _instance;
			return _instance;
		}

	public:
		RenderDevice();
		~RenderDevice();

	public:
		bool init(HINSTANCE inst, HWND hwnd);
		void release();

	public:
		inline const VkDevice& device()	const								{ return __vk_device;					}
		inline const VkSurfaceKHR& surface() const							{ return __vk_surface;					}
		inline const std::vector<VkPhysicalDevice>& gpus() const			{ return __vk_gpus;						}
		inline const VkPhysicalDeviceMemoryProperties& memory_props() const { return __vk_memory_props;				}

		inline uint32_t graphics_queue_family_index() const					{ return __graphics_queue_family_index;	}
		inline uint32_t	present_queue_family_index() const					{ return __present_queue_family_index;	}
		inline VkFormat	surface_format() const								{ return __surface_format;				}

	protected:
		bool _init_global_extension_properties(LayerProperties& layerProps);
		bool _init_global_layer_properties();
		bool _init_instance_extension_names();
		bool _init_device_extension_names();

		bool _init_instance();
		bool _init_enumerate_device();
		bool _init_surface_khr();
		bool _init_device();

	private:
		VkInstance									__vk_inst;
		std::vector<VkPhysicalDevice>				__vk_gpus;
		std::vector<VkQueueFamilyProperties>		__vk_queue_family_props;
		VkPhysicalDeviceMemoryProperties			__vk_memory_props;
		VkPhysicalDeviceProperties					__vk_gpu_props;
		VkDevice									__vk_device;
		VkSurfaceKHR								__vk_surface;

		std::vector<LayerProperties>				__layer_props_vec;
		std::vector<const char*>					__inst_extension_names;
		std::vector<const char*>					__device_extension_names;
		uint32_t									__queue_family_count;
		uint32_t									__queue_family_index;
		uint32_t									__graphics_queue_family_index;
		uint32_t									__present_queue_family_index;
		VkFormat									__surface_format;

		HINSTANCE									__window_instance;
		HWND										__window_hwnd;
		std::string									__app_short_name;
	};
}

#endif // !__RENDER_DEVICE_H__
