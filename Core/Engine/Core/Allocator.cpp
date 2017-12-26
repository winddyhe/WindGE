#include "Allocator.h"

using namespace WindGE;

void* Allocator::__alloc(size_t size, size_t alignment, VkSystemAllocationScope allocScope)
{
	return aligned_malloc(size, alignment);
}

void* WIND_CALL Allocator::__alloc(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocScope)
{
	return static_cast<Allocator*>(userData)->__alloc(size, alignment, allocScope);
}

void* Allocator::__realloc(void* original, size_t size, size_t alignment, VkSystemAllocationScope allocScope)
{
	return nullptr;
}

void* WIND_CALL Allocator::__realloc(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocScope)
{
	return static_cast<Allocator*>(userData)->__realloc(original, size, alignment, allocScope);
}

void Allocator::__free(void* mem)
{
	aligned_free(mem);
}

void WIND_CALL Allocator::__free(void* userData, void* mem)
{
	return static_cast<Allocator*>(userData)->__free(mem);
}

