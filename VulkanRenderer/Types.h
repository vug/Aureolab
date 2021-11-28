#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

// Helper to keep Vulkan handler to a buffer and it's allocation library state together
struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
};

// Helper to keep Vulkan handler to an image and it's allocation library state together
struct AllocatedImage {
	VkImage image;
	VmaAllocation allocation;
};