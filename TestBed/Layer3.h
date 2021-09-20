#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/GraphicsAPI.h"
#include "Modeling/Modeling.h"
#include "Core/ImGuiHelper.h"

#include <glad/glad.h>
//#define GLM_FORCE_CTOR_INIT /* initialize vectors and matrices */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

#include <array>
#include <iostream>
#include <vector>

class Layer3 : public Layer {
public:
    Layer3() : Layer("OBJ Loading") { }

    virtual void OnAttach() override {
        shader = Shader::Create("assets/BasicShader.glsl");
        Log::Info("attribute locations: {} {} {} {}", shader->GetAttribLocation("a_Position"), shader->GetAttribLocation("a_Normal"),
            shader->GetAttribLocation("a_TexCoord"), shader->GetAttribLocation("a_Color"));

        for (auto& objectFileName : objectFileNames) {
            std::vector<BasicVertex> vertices = LoadOBJ(std::string("assets/") + objectFileName + ".obj");
            Log::Info("{} num vertices: {}", objectFileName, vertices.size());
            VertexBuffer* vb = VertexBuffer::Create(BasicVertexAttributeSpecs);
            vb->SetVertices(vertices);

            VertexArray* va = VertexArray::Create();
            va->AddVertexBuffer(*vb);

            vertexArrays.push_back(va);
        }

        ga = GraphicsAPI::Create();
        ga->Initialize();
        ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        ga->Enable(GraphicsAbility::DepthTest);
        ga->Enable(GraphicsAbility::FaceCulling);
        ga->Disable(GraphicsAbility::Blend);
    }

    virtual void OnUpdate(float ts) override {
        float aspect = 1.0;
        glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
        glm::vec3 eye({ 0.0, 0.0, 5.0 });
        glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
        if (isModelSpinning) {
            angle += ts * 0.5f;
            model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
        }

        glm::mat4 mv = view * model;
        glm::mat4 mvp = projection * mv;
        glm::mat4 normalMatrix = glm::inverse(mv);

        ga->Clear();
        shader->Bind();
        shader->UploadUniformMat4("u_ModelViewPerspective", mvp);
        shader->UploadUniformMat4("u_ModelView", mv);
        shader->UploadUniformMat4("u_Model", model);
        shader->UploadUniformMat4("u_View", view);
        shader->UploadUniformMat4("u_NormalMatrix", mv);
        shader->UploadUniformInt("u_RenderType", renderType);
        shader->UploadUniformFloat4("u_SolidColor", solidColor);
        shader->UploadUniformFloat("u_MaxDepth", maxDepth);
        shader->UploadUniformFloat3("u_LightPosition", lightPosition);
        shader->UploadUniformFloat3("u_LightAttenuation", lightAttenuation);
        shader->UploadUniformFloat4("u_DiffuseColor", diffuseColor);
        shader->UploadUniformFloat3("u_HemisphereLightPosition", hemisphereLightPosition);
        shader->UploadUniformFloat3("u_SkyColor", skyColor);
        shader->UploadUniformFloat3("u_GroundColor", groundColor);
        ga->DrawArrayTriangles(*vertexArrays[selectedVaIndex]);
    }

    virtual void OnDetach() override {
        // optional
        delete shader;
        delete ga;
    }

    virtual void OnEvent(Event& ev) override {
        auto dispatcher = EventDispatcher(ev);
    }

    virtual void OnImGuiRender() override {
        std::vector<const char*> models;
        for (int i = 0; i < objectFileNames.size(); ++i) models.push_back(objectFileNames[i].c_str());
        ImGui::Combo("OBJ", &selectedVaIndex, models.data(), models.size());
        ImGui::SameLine(); ImGuiHelper::InfoMarker("Choose object loaded into vertex arrays from OBJ files.");

        std::vector<const char*> types;
        for (int i = 0; i < renderTypes.size(); ++i) types.push_back(renderTypes[i].c_str());
        ImGui::Combo("Render Type", &renderType, types.data(), types.size());
        ImGui::SameLine(); ImGuiHelper::InfoMarker("Choose Visualization Type.");

        ImGui::Checkbox("Spin Model", &isModelSpinning);

        if (renderType == 0) {
            ImGui::ColorEdit4("Solid Color", glm::value_ptr(solidColor));
        }
        else if (renderType == 3) {
            ImGui::DragFloat("Max Depth", &maxDepth, 1.0, 0.0f, 100.0f);
        }
        else if (renderType == 4) {
            ImGui::ColorEdit4("Diffuse Color", glm::value_ptr(diffuseColor));
            ImGui::InputFloat3("Light Position", glm::value_ptr(lightPosition));
            ImGui::InputFloat3("Light Attenuatio", glm::value_ptr(lightAttenuation));
        }
        else if (renderType == 5) {
            ImGui::InputFloat3("Hemisphere Light Position", glm::value_ptr(hemisphereLightPosition));
            ImGui::ColorEdit3("Sky Color", glm::value_ptr(skyColor));
            ImGui::ColorEdit3("Ground Color", glm::value_ptr(groundColor));
        }
    }

private:
    GraphicsAPI* ga = nullptr;
    std::vector<VertexArray*> vertexArrays;
    glm::mat4 model = glm::mat4(1.0f);
    Shader* shader = nullptr;
    float angle = 0.0f;

    std::vector<std::string> objectFileNames = { "cone", "cube", "cylinder", "plane", "sphere_ico", "sphere_ico_smooth", 
        "sphere_uv", "suzanne", "suzanne_smooth", "torus", "torus_smooth"};
    int selectedVaIndex = 8;
    std::vector<std::string> renderTypes = { "Solid Color", "Normal", "UV", "Depth", "Point Light", "Hemisphere Light"};
    int renderType = 5;
    bool isModelSpinning = true;
    glm::vec4 solidColor = { 0.8, 0.2, 0.3, 1.0 };
    float maxDepth = 100.0f;
    glm::vec3 lightPosition = { 0.0f, 3.0f, -1.0f };
    glm::vec3 lightAttenuation = { 0.0f, 1.0f, 0.0f };
    glm::vec4 diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec3 hemisphereLightPosition = { 0.0f, 1.0f, 0.0f }; // north pole
    glm::vec3 skyColor = { 0.3f, 0.4f, 0.95f };
    glm::vec3 groundColor = { 0.05f, 0.10f, 0.06f };
};