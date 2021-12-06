#pragma once

#include "Example.h"
#include "Mesh.h"
#include "Texture.h"

#include <glm/gtx/transform.hpp>

#include <chrono>
#include <memory>

class FrameData05 : public IFrameData {
public:
    //FrameData04(const FrameSyncCmd& syncCmd1) : IFrameData(syncCmd1) {} // implied default constructor
    RenderView renderView;
};

class Ex05Textures: public Example {
public:
    Ex05Textures(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
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

        VkDescriptorSetLayout singleTextureSetLayout;
        // Texture Assets
        {
            // TODO: Hide texture related DescriptorSetLayout creation behind an abstraction
            VkDescriptorSetLayoutBinding texSetBind = {};
            texSetBind.binding = 0;
            texSetBind.descriptorCount = 1;
            texSetBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            texSetBind.pImmutableSamplers = nullptr;
            texSetBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo set3info = {};
            set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            set3info.bindingCount = 1;
            set3info.flags = 0;
            set3info.pBindings = &texSetBind;
            vkCreateDescriptorSetLayout(vc.GetDevice(), &set3info, nullptr, &singleTextureSetLayout);
            destroyer.Add(singleTextureSetLayout);

            Texture texture;
            texture.LoadImageFromFile("assets/textures/texture.jpg");
            vr.UploadTexture(texture);
            vr.textures["sculpture"] = texture;

            destroyer.Add(texture.imageView);
            // TODO figure out how to add image to deletion queue from UploadTexture. Observe how texture's and newImage's addresses change etc.
            destroyer.Add(texture.newImage);
        }

        // Samplers
        VkSampler blockySampler;
        {
            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            vkCreateSampler(vc.GetDevice(), &samplerInfo, nullptr, &blockySampler);
            destroyer.Add(blockySampler);
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
        VkDescriptorPool descriptorPool = vc.CreateDescriptorPool(
            vc.GetDevice(), { 
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
                //{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 }, // Pool size allocations for examples from "Vulkan Guide"
                //{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
            }
        );
        destroyer.Add(descriptorPool);

        int framesInFlight = 2;
        for (int i = 0; i < framesInFlight; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            FrameData04 frame{ syncCmd };

            frame.renderView.Init(vc.GetDevice(), vc.GetAllocator(), descriptorPool, vc.GetDestroyer());
            frame.renderView.GetDescriptorSetLayouts();

            frameDatas.push_back(std::make_shared<FrameData04>(frame));

            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
            destroyer.Add(frame.renderView.GetCameraBuffer());
        }

        std::vector<VkDescriptorSetLayout> texturedMaterialSetLayouts = {};
        auto& renderViewSetLayouts = std::static_pointer_cast<FrameData04>(frameDatas[0])->renderView.GetDescriptorSetLayouts();
        texturedMaterialSetLayouts.insert(texturedMaterialSetLayouts.end(), renderViewSetLayouts.begin(), renderViewSetLayouts.end());
        texturedMaterialSetLayouts.push_back(singleTextureSetLayout);
        // Material Assets (aka pipelines and pipeline layouts)
        {
            VkShaderModule vertShader, fragShader;
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderViewSetLayouts, renderPass);
            vr.materials["vizNormal"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-uv-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderViewSetLayouts, renderPass);
            vr.materials["vizUV"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), texturedMaterialSetLayouts, renderPass);
            vr.materials["textured"] = Material{ pipeline, pipelineLayout };
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

            const AllocatedBuffer& camBuf = frameData->renderView.GetCameraBuffer();
            void* data;
            vmaMapMemory(vc.GetAllocator(), camBuf.allocation, &data);
            memcpy(data, &camData, sizeof(RenderView::Camera));
            vmaUnmapMemory(vc.GetAllocator(), camBuf.allocation);

            vr.DrawObjects(cmd, renderView, objects);
            });
    }

    ~Ex05Textures() {
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