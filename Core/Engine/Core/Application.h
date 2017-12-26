#pragma once
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <memory>
#include <vector>

#include "Config.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vk_sdk_platform.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "Log.h"
#include "../../Data/cube_data.h"
#include "../Render/RenderDevice.h"

namespace WindGE
{
	#define FENCE_TIMEOUT			100000000
	#define NUM_DESCRIPTOR_SETS		1
	#define NUM_SAMPLES				VK_SAMPLE_COUNT_1_BIT
	#define NUM_VIEWPORTS			1
	#define NUM_SCISSORS			NUM_VIEWPORTS

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

		typedef struct 
		{
			VkBuffer buf;
			VkDeviceMemory mem;
			VkDescriptorBufferInfo buffer_info;
		} VertexBufferData;

	public:
		Application();
		virtual ~Application();

	public:
		virtual bool init(HINSTANCE inst, HWND hwnd, int width, int height);
		virtual void update(float deltaTime);
		virtual void draw();
		virtual void resize(int w, int h);

	protected:
		VkResult _init_command_pool();
		VkResult _init_command_buffer();
		VkResult _execute_begin_command_buffer();
		VkResult _execute_end_command_buffer();
		VkResult _execute_queue_command_buffer();

		void	 _init_device_queue();

		VkResult _init_swap_chain(VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		
		bool	 _init_depth_buffer();
		bool	 _init_uniform_buffer();
		bool     _init_descriptor_pipeline_layouts(bool useTexture);
		bool	 _init_renderpass(bool includeDepth, bool clear = true, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		bool	 _init_shaders();
		bool	 _init_frame_buffers(bool includeDepth);
		bool	 _init_vertex_buffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
		bool	 _init_descriptor_pool(bool useTexture);
		bool	 _init_descriptor_set(bool useTexture);
		bool	 _init_pipeline_cache();
		bool	 _init_pipeline(bool includeDepth, bool includeVi = true);

		bool	 _draw_cube();

		bool	 _memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
		bool     _set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

		EShLanguage _find_language(const VkShaderStageFlagBits shaderType);
		bool		_glsl_to_spv(const VkShaderStageFlagBits shaderType, const char* pShader, std::vector<unsigned int> &spirv);
		void		_init_shader_resources(TBuiltInResource &resources);

		void		_init_viewports();
		void		_init_scissors();

	protected:
		RenderDevice&							__render_device;

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
		VkPipelineShaderStageCreateInfo			__vk_pipeline_shaderstages[2];
		VkFramebuffer*							__vk_framebuffers;
		VertexBufferData						__vk_vertex_buffer;
		VkVertexInputBindingDescription			__vk_vi_binding;
		VkVertexInputAttributeDescription		__vk_vi_attribs[2];
		VkDescriptorPool						__vk_descriptor_pool;
		std::vector<VkDescriptorSet>			__vk_descriptor_sets;
		VkPipelineCache							__vk_pipeline_cache;
		VkPipeline								__vk_pipeline;
		VkViewport								__vk_viewport;
		VkRect2D								__vk_scissor;

		int										__client_width;
		int										__client_height;
		uint32_t								__current_swapchain_buffer;
		uint32_t								__swapchain_image_count;

		glm::mat4								__proj_mat;
		glm::mat4								__view_mat;
		glm::mat4								__model_mat;
		glm::mat4								__clip_mat;
		glm::mat4								__mvp_mat;

		UniformData								__uniform_mvp;
	};
}

#endif // !__APPLICATION_H__

