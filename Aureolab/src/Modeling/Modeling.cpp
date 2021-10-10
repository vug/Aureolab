#include "Modeling.h"

#include "Core/Log.h"
#include "Core/Math.h"

#include <tiny_obj_loader.h>

#include <array>


// OBJ file is a compressed format. For each attribute it has indices. for example only one color value is stored if all vertices has the same color etc.
// However, vertices sent to the GPU have unique combination of attributes. Therefore it's better to JOIN all attributes for each individual vertex.
// LoadOBJ decompress OBJ into a vector of Vertex.
std::vector<BasicVertex> LoadOBJ(const std::string& filepath) { // taken from https://github.com/tinyobjloader/tinyobjloader and modified
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filepath, reader_config)) {
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
    //const tinyobj::shape_t& shape = shapes[0];
    //const tinyobj::mesh_t& mesh = shape.mesh;
    //for (auto index : mesh.indices) Log::Debug("Indices: vertex {}, normal {}, texture {}", index.vertex_index, index.normal_index, index.texcoord_index);

    std::vector<BasicVertex> vertices;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                BasicVertex vertex;

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

                // Blender does not export vertex color. For now will keep color attribute commented out.
                /*if (attrib.colors.size() > 0) {
                    tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
                    tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
                    tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
                    // alpha?
                    vertex.color = { red, green, blue, 1.0 };
                }*/
                vertices.push_back(vertex);

                //Log::Debug("vertex: ({} {} {}), ({} {} {}), ({} {}), ({} {} {} {})",
                //    vertex.position.x, vertex.position.y, vertex.position.z,
                //    vertex.normal.x, vertex.normal.y, vertex.normal.z,
                //    vertex.texCoord.x, vertex.texCoord.y,
                //    vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a
                //);
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }
    return vertices; // apparently this is move semantics, i.e. locally created vector will be moved to the scope of the caller.
}


std::vector<BasicVertex> GenerateBox(glm::vec3 dimensions) {
    glm::vec3 halfDim = dimensions * 0.5f;
    float width = halfDim.x, height = halfDim.y, depth = halfDim.z;

    // corners
    struct Vertex { glm::vec3 position; glm::vec4 color; };
    Vertex p000 = { { -width, -height, -depth }, {0.0, 0.0, 0.0, 1.0} };
    Vertex p001 = { { -width, -height, +depth }, {0.0, 0.0, 1.0, 1.0} };
    Vertex p010 = { { -width, +height, -depth }, {0.0, 1.0, 0.0, 1.0} };
    Vertex p011 = { { -width, +height, +depth }, {0.0, 1.0, 1.0, 1.0} };
    Vertex p100 = { { +width, -height, -depth }, {1.0, 0.0, 0.0, 1.0} };
    Vertex p101 = { { +width, -height, +depth }, {1.0, 0.0, 1.0, 1.0} };
    Vertex p110 = { { +width, +height, -depth }, {1.0, 1.0, 0.0, 1.0} };
    Vertex p111 = { { +width, +height, +depth }, {1.0, 1.0, 1.0, 1.0} };

    // normals
    glm::vec3 nFront = { 0.0f, 0.0f, 1.0f };
    glm::vec3 nBack = -nFront;
    glm::vec3 nLeft = { 1.0f, 0.0, 0.0 };
    glm::vec3 nRight = -nLeft;
    glm::vec3 nUp = { 0.0f, 1.0f, 0.0f };
    glm::vec3 nDown = -nUp;

    // faces (four corners in CCW, 1 normal)
    struct Face { std::array<Vertex, 4> corners; glm::vec3 normal; };
    Face fBack = { { p010, p110, p100, p000, }, nBack };
    Face fFront = { { p001, p101, p111, p011, }, nFront };
    Face fLeft = { { p110, p111, p101, p100, }, nLeft };
    Face fRight = { { p000, p001, p011, p010, }, nRight };
    Face fUp = { { p010, p011, p111, p110 }, nUp };
    Face fDown = { { p100, p101, p001, p000 }, nDown };

    std::vector<BasicVertex> vertices;
    int indices[] = { 0, 1, 2,    // triangle 1 of quad
                      0, 2, 3, }; // triangle 2 of quad
    glm::vec2 uvs[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
    for (auto& face : { fBack, fFront, fLeft, fRight, fUp, fDown }) {
        for (int ix : indices) {
            const Vertex& v = face.corners[ix];
            vertices.emplace_back<BasicVertex>(
                { v.position, face.normal, uvs[ix], v.color, }
            );
        }
    }
    return vertices;
}

std::vector<BasicVertex> GenerateTorus(float outerRadius, int outerSegments, float innerRadius, int innerSegments) {
    std::vector<BasicVertex> points;
    for (int i = 0; i < outerSegments; i++) {
        float u = (float)i / (outerSegments - 1);
        float outerAngle = Math::TAU * u;
        glm::vec3 innerCenter = glm::vec3 { cosf(outerAngle), sinf(outerAngle), 0.0f } * outerRadius;
        for (int j = 0; j < innerSegments; j++) {
            float v = (float)j / (innerSegments - 1);
            float innerAngle = Math::TAU * v;
            glm::vec3 innerPos = glm::vec3{ cosf(innerAngle) * cosf(outerAngle),  cosf(innerAngle) * sinf(outerAngle), sinf(innerAngle) } * innerRadius;

            glm::vec3 pos = innerCenter + innerPos;
            glm::vec3 norm = glm::normalize(innerPos);
            glm::vec2 uv = { u, v };
            float checker = (i % 2) ^ (j % 2);
            glm::vec4 col = glm::vec4 { 1.0, 1.0, 0.0, 1.0 } * checker + glm::vec4{ 0.0, 1.0, 1.0, 1.0 } * (1.0f - checker);

            points.emplace_back(BasicVertex { pos, norm, uv, col });
        }
    }

    std::vector<BasicVertex> vertices;
    for (size_t i = 0; i < outerSegments; i++) {
        for (size_t j = 0; j < innerSegments; j++) {
            size_t i1 = (i + 1) % outerSegments;
            size_t j1 = (j + 1) % innerSegments;
            const BasicVertex& p1 = points[i * innerSegments + j];
            const BasicVertex& p2 = points[i * innerSegments + j1];
            const BasicVertex& p3 = points[i1 * innerSegments + j];
            const BasicVertex& p4 = points[i1 * innerSegments + j1];
            vertices.push_back(p3); // triangle-1
            vertices.push_back(p2);
            vertices.push_back(p1);

            vertices.push_back(p2); // triangle-2
            vertices.push_back(p3);
            vertices.push_back(p4);
        }
    }
    
    return vertices;
}
;
