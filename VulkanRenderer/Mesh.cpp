#include "Mesh.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>

VertexInputDescription Vertex::GetVertexDescription() {
	VertexInputDescription description;

	// 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = mainBinding.binding;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, position);

	VkVertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = mainBinding.binding;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Vertex, normal);

	VkVertexInputAttributeDescription texCoordAttribute = {};
	texCoordAttribute.binding = mainBinding.binding;
	texCoordAttribute.location = 2;
	texCoordAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	texCoordAttribute.offset = offsetof(Vertex, texCoord);

	VkVertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = mainBinding.binding;
	colorAttribute.location = 3;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, color);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(texCoordAttribute);
	description.attributes.push_back(colorAttribute);
	return description;
}

std::vector<VkPushConstantRange> MeshPushConstants::GetPushConstantRanges() {
	std::vector<VkPushConstantRange> pushConstantRanges;
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(MeshPushConstants::PushConstant1);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRanges.push_back(pushConstant);
	return pushConstantRanges;
}

bool Mesh::LoadFromOBJ(const char* filename) {
	Log::Debug("Loading OBJ file: {}...", filename);
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	//materials contains the information about the material of each shape, but we won't use it.
	std::vector<tinyobj::material_t> materials;

	//error and warning output from the load function
	std::string warn;
	std::string err;

	//load the OBJ file
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
	//make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty()) {
		Log::Warning("{}", warn);
	}
	//if we have any error, print it to the console, and break the mesh loading.
	//This happens if the file can't be found or is malformed
	if (!err.empty()) {
		Log::Error("{}", err);
		return false;
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		Log::Debug("\tShape[{}] has {} faces.", s, shapes[s].mesh.num_face_vertices.size());
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			//hardcode loading to triangles
			int fv = 3;

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				Vertex new_vert;

				//vertex position
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				new_vert.position = { vx, vy, vz };

				//vertex normal
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					new_vert.normal = { nx, ny, nz };
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					new_vert.texCoord = { tx, ty };
				}

				// OBJ format does not have color information in it (it relies on accompanying MTL files to assign diffuse and specular colors to faces)
				// However, some software extends OBJ format by adding 3 more floats per vertex (next to position).
				// I tested that if color info is not there, vertex color becomes white.
				{
					tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
					new_vert.color = { red, green, blue, 1.0f };
				}

				vertices.push_back(new_vert);
			}
			index_offset += fv;
		}
	}

	return true;
}

void Mesh::MakeTriangle() {
	vertices = {
		{ { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, { 1.f, 0.f, 0.0f, 1.0f } },
		{ {  0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }, { 0.f, 1.f, 0.0f, 1.0f } },
		{ {  0.0f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.5f, 1.0f }, { 0.f, 0.f, 1.0f, 1.0f } },
	};
}

void Mesh::MakeQuad() {
	vertices = {
		{ { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, { 1.f, 0.f, 0.0f, 1.0f } },
		{ {  0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f }, { 0.f, 1.f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }, { 0.f, 0.f, 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, { 1.f, 0.f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f }, { 0.f, 0.f, 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f }, { 1.f, 1.f, 1.0f, 1.0f } },
	};
}
