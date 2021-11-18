#pragma once
#include "VulkanContext.h"

#include "IResizable.h"

#include <string>
#include <vector>

class VulkanRenderer : public IResizable {
public:
	VulkanRenderer(VulkanContext& context);
	~VulkanRenderer();

	virtual void OnResize(int width, int height) override;

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkRenderPass CreateRenderPass();
	void CreateExampleGraphicsPipeline(const std::string& vertFilename, const std::string& fragFilename, VkRenderPass& renderPass);

private:
	// declaring as reference prevents it from being destroyed with Renderer
	VulkanContext& vc;

	// Resources that needs to be destroyed at the end
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
};