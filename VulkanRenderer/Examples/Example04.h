#pragma once

#include "Example.h"

#include <glm/gtx/transform.hpp>

#include <chrono>
#include <memory>

class FrameData04 : public IFrameData {
public:
    //FrameData04(const FrameSyncCmd& syncCmd1) : IFrameData(syncCmd1) {} // implied default constructor
    RenderView renderView;
};

class Ex04DescriptorSets : public Example {
public:
    Ex04DescriptorSets(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
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

        // Descriptors
        VkDescriptorPool descriptorPool = vc.CreateDescriptorPool(
            vc.GetDevice(), { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 } }
        );
        destroyer.Add(descriptorPool);

        int framesInFlight = 2;
        for (int i = 0; i < framesInFlight; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            FrameData04 frame{ syncCmd };

            frame.renderView.Init(vc.GetDevice(), vc.GetAllocator(), descriptorPool, vc.GetDestroyer());

            frameDatas.push_back(std::make_shared<FrameData04>(frame));

            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
            destroyer.Add(frame.renderView.GetCameraBuffer());
        }

        auto& descriptorSetLayouts = std::static_pointer_cast<FrameData04>(frameDatas[0])->renderView.GetDescriptorSetLayouts();
        // Material Assets (aka pipelines and pipeline layouts)
        {
            VkShaderModule vertShader, fragShader;
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), descriptorSetLayouts, vc.swapchainRenderPass);
            vr.materials["vizNormal"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-uv-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), descriptorSetLayouts, vc.swapchainRenderPass);
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

    void OnRender(float time, float delta) {
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil.depth = 1.0f;

        glm::vec3 camPos = { 0.f, 0.f, -2.f };

        for (auto& obj : objects) {
            obj.transform = glm::rotate(obj.transform, delta, { 0, 1, 0 });
        }

        vc.drawFrame(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), vc.swapchainRenderPass, frameDatas, vc.swapchainFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd, uint32_t frameNo) {
            auto frameData = std::static_pointer_cast<FrameData04>(frameDatas[frameNo]);

            RenderView& renderView = frameData->renderView;
            renderView.camera = {
                glm::translate(glm::mat4(1.f), camPos),
                glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f),
            };
            renderView.camera.projection[1][1] *= -1;

            RenderView::Camera camData;
            camData.projection = renderView.camera.projection;
            camData.view = renderView.camera.view;

            const AllocatedBuffer& camBuf = frameData->renderView.GetCameraBuffer();
            void* data;
            vmaMapMemory(vc.GetAllocator(), camBuf.allocation, &data);
            memcpy(data, &camData, sizeof(RenderView::Camera));
            vmaUnmapMemory(vc.GetAllocator(), camBuf.allocation);

            vr.DrawObjects(cmd, renderView, objects);
        });
    }

    ~Ex04DescriptorSets() {
        for (auto& frameData : frameDatas) {
            const FrameSyncCmd& frameSyncCmd = frameData->GetFrameSyncCmdData();
            vkFreeCommandBuffers(vc.GetDevice(), frameSyncCmd.commandPool, 1, &frameSyncCmd.mainCommandBuffer);
        }
    }
private:
    std::vector<std::shared_ptr<IFrameData>> frameDatas;

    std::vector<RenderObject> objects;
};