#pragma once
#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include "Config.h"
#include <assert.h>

#include "vulkan\vulkan.h"

namespace WindGE
{
	inline static void* aligned_malloc(size_t size, size_t alignment)
	{
		// check alignment is 2^n
		assert(!(alignment & (alignment - 1)));

		size_t offset = sizeof(void*) + (--alignment);

		char* p = static_cast<char*>(malloc(offset + size));
		if (!p) return nullptr;

		void* rp = reinterpret_cast<void*>(reinterpret_cast<size_t>(p + offset)&(~alignment));
		static_cast<void**>(rp)[-1] = p;

		return rp;
	}

	inline static void* aligned_realloc(void* data, size_t size, size_t alignment)
	{
		// check alignment is 2^n
		assert(!(alignment & (alignment - 1)));

		size_t offset = sizeof(void*) + (--alignment);
		
		char* p = static_cast<char*>(data);
		if (!p) p = static_cast<char*>(malloc(offset + size));
		if (!p) return nullptr;

		void* rp = reinterpret_cast<void*>(reinterpret_cast<size_t>(p + offset)&(~alignment));
		static_cast<void**>(rp)[-1] = p;

		return rp;
	}

	inline static void aligned_free(void* p)
	{
		free(static_cast<void**>(p)[-1]);
	}

	class Allocator
	{
	public:
		inline operator VkAllocationCallbacks() const 
		{
			VkAllocationCallbacks allocCallbacks;
			allocCallbacks.pUserData = (void*)this;
			allocCallbacks.pfnAllocation = __alloc;
			allocCallbacks.pfnReallocation = __realloc;
			allocCallbacks.pfnFree = __free;
			allocCallbacks.pfnInternalAllocation = nullptr;
			allocCallbacks.pfnInternalFree = nullptr;
			return allocCallbacks;
		}

	private:
		static void* WIND_CALL __alloc(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocScope);
		static void* WIND_CALL __realloc(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocScope);
		static void  WIND_CALL __free(void* userData, void* mem);

		void* __alloc(size_t size, size_t alignment, VkSystemAllocationScope allocScope);
		void* __realloc(void* original, size_t size, size_t alignment, VkSystemAllocationScope allocScope);
		void  __free(void* mem);
	};

}

#endif // !__ALLOCATOR_H__
