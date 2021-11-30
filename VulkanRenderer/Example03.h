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

        // Material Assets (aka pipelines and pipeline layouts)
        {
            VkShaderModule vertShader, fragShader;
            VkPipeline pipeline;
            VkPipelineLayout pipelineLayout;

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/default-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderPass);
            vr.materials["vizNormal"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);

            vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/default-vert.spv"));
            fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-uv-frag.spv"));
            std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderPass);
            vr.materials["vizUV"] = Material{ pipeline, pipelineLayout };
            destroyer.Add(std::vector{ vertShader, fragShader });
            destroyer.Add(pipelineLayout);
            destroyer.Add(pipeline);
        }
            
        cmdBuf = vr.CreateCommandBuffer();
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
        RenderView renderView = {
            glm::translate(glm::mat4(1.f), camPos),
            glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f),
        };
        renderView.projection[1][1] *= -1;

        for (auto& obj : objects) { 
            obj.transform = glm::rotate(obj.transform, delta.count(), { 0, 1, 0 }); 
        }

        vc.drawFrameBlocked(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](VkCommandBuffer& cmd) {
            vr.DrawObjects(cmd, renderView, objects);
        });
    }

    ~Ex03SceneManagement() {
        vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    }
private:
    VkRenderPass renderPass;
    VkCommandBuffer cmdBuf;
    std::vector<VkFramebuffer> presentFramebuffers;

    std::vector<RenderObject> objects;
};