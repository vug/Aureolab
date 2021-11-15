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

	void CreateExampleGraphicsPipeline(const std::string& vertFilename, const std::string& fragFilename);

private:
	// Resources that needs to be destroyed at the end
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger = nullptr;
	VkDevice device;
	VkSwapchainKHR swapChain;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// SwapChain related info that is used in creation of GraphicsPipeline
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D swapExtent; // used in GraphicsPipeline creation

	//
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};