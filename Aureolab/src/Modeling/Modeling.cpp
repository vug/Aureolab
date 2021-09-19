#include "Modeling.h"

#include "Core/Log.h"

#include <tiny_obj_loader.h>

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
