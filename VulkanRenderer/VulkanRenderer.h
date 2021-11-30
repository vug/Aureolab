#pragma once
#include "VulkanContext.h"

#include "IResizable.h"
#include "Mesh.h"

#include <string>
#include <tuple>
#include <vector>

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transform;
};

class VulkanRenderer {
public:
	VulkanRenderer(VulkanContext& context);
	~VulkanRenderer();

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkRenderPass CreateRenderPass();
	std::tuple<VkPipeline, VkPipelineLayout> CreateSinglePassGraphicsPipeline(
		VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule,
		const VertexInputDescription& vertDesc,
		const std::vector<VkPushConstantRange>& pushConstantRanges,
		VkRenderPass& renderPass
	);

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void UploadMesh(Mesh& mesh);

	static glm::mat4 MakeTransform(const glm::vec3& translate, const glm::vec3& axis, float angle, const glm::vec3& scale);
private:
	// declaring as reference prevents it from being destroyed with Renderer
	VulkanContext& vc;

	// Resources that needs to be destroyed at the end
	std::vector<VkFramebuffer> swapChainFramebuffers;
};