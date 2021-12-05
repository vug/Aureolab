#pragma once

#include "Example.h"

#include <glm/gtx/transform.hpp>

#include <chrono>
#include <memory>

class FrameData04 : public IFrameData {
public:
    //FrameData04(const FrameSyncCmd& syncCmd1) : IFrameData(syncCmd1) {} // implied default constructor
    AllocatedBuffer cameraBuffer;
    RenderView renderView;
};

class Ex04DescriptorSets : public Example {
public:
    Ex04DescriptorSets(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
        // Mesh Assets
        {
            Mesh mesh;
            mesh.MakeTriangle();
            vr.UploadMesh(mesh);
            vr.meshes["triangle"] = mesh; // copy operation

            mesh.MakeQuad();
            vr.UploadMesh(mesh);
            vr.meshes["quad"] = mesh;

            mesh.LoadFromOBJ("assets/models/suzanne.obj");
            vr.UploadMesh(mesh);
            vr.meshes["monkey_flat"] = mesh;

            destroyer.Add(std::vector{ vr.meshes["triangle"].vertexBuffer, vr.meshes["quad"].vertexBuffer, vr.meshes["monkey_flat"].vertexBuffer });
        }

        renderPass = vr.CreateRenderPass();
        destroyer.Add(renderPass);
        AllocatedImage depthImage;
        VkImageView depthImageView;
        std::tie(presentFramebuffers, depthImageView, depthImage) = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), vc.GetAllocator(), renderPass, vc.GetSwapchainInfo());
        destroyer.Add(presentFramebuffers);
        destroyer.Add(depthImageView);
        destroyer.Add(depthImage);

        // Descriptors
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = RenderView::CreateDescriptorSetLayouts(vc.GetDevice(), vc.GetDestroyer());
        VkDescriptorPool descriptorPool;
        {
            //create a descriptor pool that will hold 10 uniform buffers
            std::vector<VkDescriptorPoolSize> sizes = {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.maxSets = 10;
            pool_info.poolSizeCount = (uint32_t)sizes.size();
            pool_info.pPoolSizes = sizes.data();
            vkCreateDescriptorPool(vc.GetDevice(), &pool_info, nullptr, &descriptorPool);
            destroyer.Add(descriptorPool);
        }

        int framesInFlight = 2;
        for (int i = 0; i < framesInFlight; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            FrameData04 frame{ syncCmd };

            frame.cameraBuffer = vc.CreateAllocatedBuffer(vc.GetAllocator(), sizeof(RenderView::Camera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            allocInfo.pSetLayouts = descriptorSetLayouts.data();
            vkAllocateDescriptorSets(vc.GetDevice(), &allocInfo, &frame.renderView.descriptorSet);

            VkDescriptorBufferInfo binfo;
            binfo.buffer = frame.cameraBuffer.buffer;
            binfo.offset = 0;
            binfo.range = sizeof(RenderView::Camera);
            VkWriteDescriptorSet setWrite = {};
            setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrite.dstBinding = 0;
            setWrite.dstSet = frame.renderView.descriptorSet;
            setWrite.descriptorCount = 1;
            setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            setWrite.pBufferInfo = &binfo;
            vkUpdateDescriptorSets(vc.GetDevice(), 1, &setWrite, 0, nullptr);

            frameDatas.push_back(std::make_shared<FrameData04>(frame));

            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
            destroyer.Add(frame.cameraBuffer);
        }

        // Material Assets (aka pipelines and pipeline layouts)
        {
            VkShaderModule vertShader, fragShader;
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), descriptorSetLayouts, renderPass);
            vr.materials["vizNormal"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-uv-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), descriptorSetLayouts, renderPass);
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

        for (auto& obj : objects) {
            obj.transform = glm::rotate(obj.transform, delta.count(), { 0, 1, 0 });
        }

        vc.drawFrame(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), renderPass, frameDatas, presentFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd, uint32_t frameNo) {
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

            const AllocatedBuffer& camBuf = frameData->cameraBuffer;
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
    VkRenderPass renderPass;
    std::vector<std::shared_ptr<IFrameData>> frameDatas;
    std::vector<VkFramebuffer> presentFramebuffers;

    std::vector<RenderObject> objects;
};