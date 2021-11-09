#pragma once
#include "Window.h"

#include "IResizable.h"

#include <vector>

class VulkanRenderer : public IResizable {
public:
	VulkanRenderer(Window& win);
	~VulkanRenderer();

	virtual void OnResize(int width, int height) override;
private:
	VkInstance instance;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};