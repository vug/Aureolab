#include "Allocator.h"

#include "Core/Log.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace vr {
	AllocatorBuilder::AllocatorBuilder(const Device& device) 
		: device(device) {
        
        info.physicalDevice = device.physicalDevice;
        info.device = device;
        info.instance = device.physicalDevice.instance;
	}

    // ---------------------------------

    Allocator::Allocator(const AllocatorBuilder& builder)
        : builder(builder), device(builder.device) {
        Log::Debug("Creating Memory Allocator...");
        vmaCreateAllocator(&builder.info, &handle);
    }

    Allocator::~Allocator() {
        Log::Debug("Destroying Memory Allocator...");
        vmaDestroyAllocator(handle);
    }
}