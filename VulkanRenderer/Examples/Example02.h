#pragma once

#include "Example.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#include <chrono>

class Ex02VertexBufferInput : public Example {
public:
    Ex02VertexBufferInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {
        triangleMesh.MakeTriangle();
        vr.UploadMeshCpuToGpu(triangleMesh);
        destroyer.Add(triangleMesh.vertexBuffer);
        quadMesh.MakeQuad();
        vr.UploadMeshCpuToGpu(quadMesh);
        destroyer.Add(quadMesh.vertexBuffer);
        monkeyMesh.LoadFromOBJ("assets/models/suzanne.obj");
        vr.UploadMeshCpuToGpu(monkeyMesh);
        destroyer.Add(monkeyMesh.vertexBuffer);

        for (int i = 0; i < 1; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            frameSyncCmds.push_back(std::make_shared<IFrameData>(syncCmd));
            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
        }

        VkShaderModule vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-vert.spv"));
        VkShaderModule fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-frag.spv"));
        destroyer.Add(vertShader);
        destroyer.Add(fragShader);
        std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), {}, vc.swapchainRenderPass);
        destroyer.Add(pipelineLayout);
        destroyer.Add(pipeline);
    }

    void OnRender(float time, float delta) {
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil.depth = 1.0f;
        vc.drawFrame(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), vc.swapchainRenderPass, frameSyncCmds, vc.swapchainFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd, uint32_t frameNo) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            Mesh mesh = int(time / 2.0f) % 2 == 0 ? monkeyMesh : quadMesh;

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

            glm::vec3 camPos = { 0.f, 0.f, -2.f };
            glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
            projection[1][1] *= -1;
            glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, time, glm::vec3(0, 1, 0));
            glm::mat4 mvp = projection * view * model;
            MeshPushConstants::PushConstant1 constants;
            constants.modelViewProjection = mvp;
            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);

            vkCmdDraw(cmd, (uint32_t)mesh.vertices.size(), 1, 0, 0);
        });
    }

    ~Ex02VertexBufferInput() {
        for (auto& frameSyncCmd : frameSyncCmds) {
            vkFreeCommandBuffers(vc.GetDevice(), frameSyncCmd->GetFrameSyncCmdData().commandPool, 1, &frameSyncCmd->GetFrameSyncCmdData().mainCommandBuffer);
        }
    }
private:
    std::vector<std::shared_ptr<IFrameData>> frameSyncCmds;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    Mesh triangleMesh, quadMesh, monkeyMesh;
};