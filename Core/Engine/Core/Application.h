#pragma once
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <memory>
#include <vector>

#include "Config.h"
#include "vulkan\vulkan.hpp"

namespace WindGE
{
	class WIND_CORE_API Application
	{
	public:
		Application();
		virtual ~Application();

	public:
		virtual bool init();
		virtual void update();
		virtual void draw();
		virtual void resize(int w, int h);

	protected:
		VkInstance								__vk_inst;
		std::vector<VkPhysicalDevice>			__vk_gpus;
		std::vector<VkQueueFamilyProperties>	__vk_queue_family_props;
		VkDevice								__vk_device;
		VkCommandPool							__vk_cmd_pool;
		VkCommandBuffer							__vk_cmd_buffer;
	};
}

#endif // !__APPLICATION_H__

