#pragma once
#include "VulkanContext.h"

#include "IResizable.h"
#include "Mesh.h"
#include "Texture.h"

#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet textureSet = { VK_NULL_HANDLE };
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
	void Init(const VkDevice& device, const VmaAllocator& allocator, const VkDescriptorPool& pool, VulkanDestroyer& destroyer);
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
	VulkanRenderer(const VulkanContext& context);
	~VulkanRenderer();

	std::tuple<VkPipeline, VkPipelineLayout> CreateSinglePassGraphicsPipeline(
		VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule,
		const VertexInputDescription& vertDesc,
		const std::vector<VkPushConstantRange>& pushConstantRanges,
		const std::vector<VkDescriptorSetLayout>& descSetLayouts,
		VkRenderPass& renderPass,
		// Params
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL
	);

	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	// Uses VMA_MEMORY_USAGE_CPU_TO_GPU type of memory which is relatively slow
	void UploadMeshCpuToGpu(Mesh& mesh);
	// Uses VMA_MEMORY_USAGE_GPU_ONLY memory via an intermediate staging buffer
	void UploadMesh(Mesh& mesh);
	// Upload pixels into shader readable GPU memory and create an imageview
	void UploadTexture(Texture& texture);

	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;
	std::unordered_map<std::string, Texture> textures;

	static glm::mat4 MakeTransform(const glm::vec3& translate, const glm::vec3& axis, float angle, const glm::vec3& scale);
	void DrawObjects(VkCommandBuffer cmd, RenderView& renderView, std::vector<RenderObject> objects);

private:
	// declaring as reference prevents it from being destroyed with Renderer
	const VulkanContext& vc;

	ImmediateCommandSubmitter imCmdSubmitter;

	// Resources that needs to be destroyed at the end
	std::vector<VkFramebuffer> swapChainFramebuffers;
};