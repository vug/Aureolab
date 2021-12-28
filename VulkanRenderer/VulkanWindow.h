#pragma once

#include "IResizable.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Window for a Vulkan App
class VulkanWindow {
public:
	VulkanWindow();
	~VulkanWindow();

	const char** GetVulkanExtensions() const { return glfwVulkanExtensions; }
	uint32_t GetVulkanExtensionCount() const { return glfwVulkanExtensionCount; }
	void GetFramebufferSize(int* width, int* height) const;
	bool ShouldClose();
	void PollEvents();

	void SetUserPointer(void* ptr);

	VkResult CreateSurface(VkInstance instance, VkSurfaceKHR* surface) const;
private:
	GLFWwindow* window;
	uint32_t glfwVulkanExtensionCount;
	const char** glfwVulkanExtensions;
};