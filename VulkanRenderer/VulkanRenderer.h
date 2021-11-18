#pragma once
#include "VulkanContext.h"

#include "IResizable.h"

#include <string>
#include <tuple>
#include <vector>

class VulkanRenderer : public IResizable {
public:
	VulkanRenderer(VulkanContext& context);
	~VulkanRenderer();

	virtual void OnResize(int width, int height) override;

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkRenderPass CreateRenderPass();
	VkFramebuffer CreateFramebuffer(VkRenderPass& renderPass, const VkImageView& imageView, const VkExtent2D& extent);
	std::tuple<VkPipeline, VkPipelineLayout> CreateSinglePassGraphicsPipeline(VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, VkRenderPass& renderPass);

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

private:
	// declaring as reference prevents it from being destroyed with Renderer
	VulkanContext& vc;

	// Resources that needs to be destroyed at the end
	std::vector<VkFramebuffer> swapChainFramebuffers;
};