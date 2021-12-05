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
public:
	struct Camera {
		glm::mat4 view;
		glm::mat4 projection;
	};

	// Create DescriptorSetLayouts, allocate and update DescriptorSets
	void Init(const VkDevice& device, const VmaAllocator& allocator, const VkDescriptorPool& pool, VulkanDestroyer& destroyer) {
		cameraBuffer = VulkanContext::CreateAllocatedBuffer(allocator, sizeof(RenderView::Camera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		descriptorSetLayouts = CreateDescriptorSetLayouts(device, destroyer);
		descriptorSet = AllocateAndUpdateDescriptorSet(device, pool, descriptorSetLayouts, cameraBuffer);
	}

	const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return descriptorSetLayouts; }
	const VkDescriptorSet& GetDescriptorSet() const { return descriptorSet; }
	const AllocatedBuffer& GetCameraBuffer() const { return cameraBuffer; }

	Camera camera;

private:
	AllocatedBuffer cameraBuffer;
	VkDescriptorSet descriptorSet;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

	static std::vector<VkDescriptorSetLayout> CreateDescriptorSetLayouts(const VkDevice& device, VulkanDestroyer& destroyer);
	static VkDescriptorSet AllocateAndUpdateDescriptorSet(const VkDevice& device, const VkDescriptorPool& pool, const std::vector<VkDescriptorSetLayout>& layouts, const AllocatedBuffer& cameraBuffer);
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
		const std::vector<VkDescriptorSetLayout>& descSetLayouts,
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