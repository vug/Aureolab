#pragma once

#include "IResizable.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Window for a Vulkan App
class Window {
public:
	Window();
	~Window();

	const char** GetVulkanExtensions() const { return glfwVulkanExtensions; }
	uint32_t GetVulkanExtensionCount() const { return glfwVulkanExtensionCount; }

	void SetUserPointer(void* ptr);
private:
	GLFWwindow* window;
	uint32_t glfwVulkanExtensionCount;
	const char** glfwVulkanExtensions;
};