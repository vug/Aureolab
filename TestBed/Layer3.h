#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Renderer/GraphicsAPI.h"

#include <glad/glad.h>
//#define GLM_FORCE_CTOR_INIT /* initialize vectors and matrices */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

#include <array>
#include <iostream>

class Layer3 : public Layer {
public:
    Layer3() : Layer("OBJ Loading") { }

    virtual void OnAttach() override {
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
            glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        };

        std::string inputfile = "assets/torus.obj";
        tinyobj::ObjReaderConfig reader_config;
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(inputfile, reader_config)) {
            if (!reader.Error().empty()) {
                Log::Error("TinyObjReader: {}", reader.Error());
            }
        }
        if (!reader.Warning().empty()) {
            Log::Warning("TinyObjReader: {}", reader.Warning());
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        if (shapes.size() != 1) {
            Log::Warning("Number of shapes in OBJ file is not equal to 1, but {}", shapes.size());
        }
        auto& materials = reader.GetMaterials();
        const tinyobj::shape_t& shape = shapes[0];
        const tinyobj::mesh_t& mesh = shape.mesh;
        for (auto index : mesh.indices) {
            Log::Debug("Indices: vertex {}, normal {}, texture {}", index.vertex_index, index.normal_index, index.texcoord_index);
        }

        std::vector<Vertex> vertices;

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    Vertex vertex;

                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                    vertex.position = { vx, vy, vz };
                    //Log::Info("Vertex[{}] = ({}, {}, {})", idx.vertex_index, vx, vy, vz);

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                        vertex.normal = { nx, ny, nz };
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0) {
                        tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                        vertex.texCoord = { tx, ty };
                    }
                    //if (attrib.colors.size() > 0) {
                    //    tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
                    //    tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
                    //    tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
                    //    // alpha?
                    //    vertex.color = { red, green, blue, 1.0 };
                    //}
                    vertices.push_back(vertex);
                    Log::Debug("vertex: ({} {} {}), ({} {} {}), ({} {}), ({} {} {} {})",
                        vertex.position.x, vertex.position.y, vertex.position.z,
                        vertex.normal.x, vertex.normal.y, vertex.normal.z,
                        vertex.texCoord.x, vertex.texCoord.y,
                        vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a
                    );
                }
                index_offset += fv;

                // per-face material
                shapes[s].mesh.material_ids[f];
            }
        }

        Log::Info("num vertices: {}", vertices.size());

        shader = Shader::Create("assets/BasicShader.glsl");

        Log::Info("attribute locations: {} {} {} {}", shader->GetAttribLocation("a_Position"), shader->GetAttribLocation("a_Normal"),
            shader->GetAttribLocation("a_TexCoord"), shader->GetAttribLocation("a_Color"));

        std::vector<VertexAttributeSpecification> specs = {
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Normal"), VertexAttributeSemantic::Normal, VertexAttributeType::float32, 3, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_TexCoord"), VertexAttributeSemantic::UV, VertexAttributeType::float32, 2, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Color"), VertexAttributeSemantic::Color, VertexAttributeType::float32, 4, false},
        };

        vb = VertexBuffer::Create(specs);
        vb->SetVertices(vertices);

        va = VertexArray::Create();
        va->AddVertexBuffer(*vb);

        ga = GraphicsAPI::Create();
        ga->Initialize();
        ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        ga->Enable(GraphicsAbility::DepthTest);
        ga->Enable(GraphicsAbility::FaceCulling);
        ga->Disable(GraphicsAbility::Blend);
    }

    virtual void OnUpdate(float ts) override {
        float aspect = 1.0;
        bool isModelSpinning = true;
        glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
        glm::vec3 eye({ 0.0, 0.0, 5.0 });
        glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
        if (isModelSpinning) {
            angle += ts * 0.5f;
            model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
        }

        glm::mat4 mvp = projection * view * model;

        ga->Clear();
        shader->Bind();
        shader->UploadUniformMat4("MVP", mvp);
        ga->DrawArrayTriangles(*va);
    }

    virtual void OnDetach() override {
        // optional
        delete vb;
        delete ib;
        delete va;
        delete shader;
        delete ga;
    }

    virtual void OnEvent(Event& ev) override {
        auto dispatcher = EventDispatcher(ev);
    }

    virtual void OnImGuiRender() override {}

private:
    GraphicsAPI* ga = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    VertexArray* va = nullptr;
    std::vector<unsigned int> indices = {};
    glm::mat4 model = glm::mat4(1.0f);
    Shader* shader = nullptr;
    float angle = 0.0f;
};