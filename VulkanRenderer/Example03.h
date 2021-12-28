#pragma once

#include "Example.h"

#include <glm/gtx/transform.hpp>

#include <chrono>

class Ex03SceneManagement : public Example {
public:
    Ex03SceneManagement(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
        // Mesh Assets
        {
            Mesh mesh;
            mesh.MakeTriangle();
            vr.UploadMeshCpuToGpu(mesh);
            vr.meshes["triangle"] = mesh; // copy operation

            mesh.MakeQuad();
            vr.UploadMeshCpuToGpu(mesh);
            vr.meshes["quad"] = mesh;

            mesh.LoadFromOBJ("assets/models/suzanne.obj");
            vr.UploadMeshCpuToGpu(mesh);
            vr.meshes["monkey_flat"] = mesh;

            destroyer.Add(std::vector{ vr.meshes["triangle"].vertexBuffer, vr.meshes["quad"].vertexBuffer, vr.meshes["monkey_flat"].vertexBuffer });
        }

        int framesInFlight = 2;
        for (int i = 0; i < framesInFlight; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            frameSyncCmds.push_back(std::make_shared<IFrameData>(syncCmd));
            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
        }

        // Material Assets (aka pipelines and pipeline layouts)
        {
            VkShaderModule vertShader, fragShader;
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/default-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), {}, vc.swapchainRenderPass);
            vr.materials["vizNormal"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/default-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-uv-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), {}, vc.swapchainRenderPass);
            vr.materials["vizUV"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);
        }
            
        objects = {
            { &vr.meshes["monkey_flat"], &vr.materials["vizNormal"], glm::translate(glm::mat4(1.0f), { -1.0f, 0.0, 0.0 }) },
            { &vr.meshes["quad"], &vr.materials["vizUV"], glm::translate(glm::mat4(1.0f), { 1.0f, 0.0, 0.0 }) },
        };
    }

    void OnRender() {
        static auto t0 = std::chrono::system_clock::now();
        static auto prevTime = std::chrono::duration<float>(0.0f);
        std::chrono::duration<float> time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t0);
        std::chrono::duration<float> delta = time - prevTime;
        prevTime = time;

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil.depth = 1.0f;

        glm::vec3 camPos = { 0.f, 0.f, -2.f };
        RenderView renderView;
        renderView.camera = {
            glm::translate(glm::mat4(1.f), camPos),
            glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f),
        };
        renderView.camera.projection[1][1] *= -1;

        for (auto& obj : objects) { 
            obj.transform = glm::rotate(obj.transform, delta.count(), { 0, 1, 0 }); 
        }

        vc.drawFrame(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), vc.swapchainRenderPass, frameSyncCmds, vc.swapchainFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd, uint32_t frameNo) {
            vr.DrawObjects(cmd, renderView, objects);
        });
    }

    ~Ex03SceneManagement() {
        for (auto& frameSyncCmd : frameSyncCmds) {
            vkFreeCommandBuffers(vc.GetDevice(), frameSyncCmd->GetFrameSyncCmdData().commandPool, 1, &frameSyncCmd->GetFrameSyncCmdData().mainCommandBuffer);
        }
    }
private:
    std::vector<std::shared_ptr<IFrameData>> frameSyncCmds;

    std::vector<RenderObject> objects;
};