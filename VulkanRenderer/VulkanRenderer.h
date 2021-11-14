#pragma once
#include "Window.h"

#include "IResizable.h"

#include <string>
#include <vector>

class VulkanRenderer : public IResizable {
public:
	VulkanRenderer(Window& win);
	~VulkanRenderer();

	virtual void OnResize(int width, int height) override;

	VkPipeline CreateExampleGraphicsPipeline(const std::string& vertFilename, const std::string& fragFilename);
private:
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkDebugUtilsMessengerEXT debugMessenger = nullptr;
	VkSwapchainKHR swapChain;
	std::vector<VkImageView> swapChainImageViews;

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};