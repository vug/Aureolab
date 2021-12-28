#pragma once

#include "Device.h"

#include <vk_mem_alloc.h>

namespace vr {
	class AllocatorBuilder {
	public:
		AllocatorBuilder(const Device& device);
		const Device& device;
		VmaAllocatorCreateInfo info = {};
	};

	struct Allocator {
		Allocator(const AllocatorBuilder& builder);
		~Allocator();
		AllocatorBuilder builder;
		VmaAllocator handle = VK_NULL_HANDLE;

		const Device& device;

		operator VmaAllocator () const { return handle; }
		operator VmaAllocator* () { return &handle; }
		operator VmaAllocator& () { return handle; }
		operator const VmaAllocator& () const { return handle; }
	};
}