#pragma once
#include "VulkanContext.h"

#include "IResizable.h"
#include "Mesh.h"

#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transform;
};

struct RenderView {
	glm::mat4 view;
	glm::mat4 projection;
};

class VulkanRenderer {
public:
	VulkanRenderer(VulkanContext& context);
	~VulkanRenderer();

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

	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;

	static glm::mat4 MakeTransform(const glm::vec3& translate, const glm::vec3& axis, float angle, const glm::vec3& scale);
	void DrawObjects(VkCommandBuffer cmd, RenderView& renderView, std::vector<RenderObject> objects);

private:
	// declaring as reference prevents it from being destroyed with Renderer
	VulkanContext& vc;

	// Resources that needs to be destroyed at the end
	std::vector<VkFramebuffer> swapChainFramebuffers;
};