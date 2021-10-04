#pragma once
#include "Renderer/VertexArray.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cereal/cereal.hpp>

#include <string>

// For serialization of glm data structures
namespace glm {
	template<class Archive> void serialize(Archive& archive, glm::vec2& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y)); }
	template<class Archive> void serialize(Archive& archive, glm::vec3& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z)); }
	template<class Archive> void serialize(Archive& archive, glm::vec4& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w)); }
	template<class Archive> void serialize(Archive& archive, glm::ivec2& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y)); }
	template<class Archive> void serialize(Archive& archive, glm::ivec3& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z)); }
	template<class Archive> void serialize(Archive& archive, glm::ivec4& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w)); }
	template<class Archive> void serialize(Archive& archive, glm::uvec2& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y)); }
	template<class Archive> void serialize(Archive& archive, glm::uvec3& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z)); }
	template<class Archive> void serialize(Archive& archive, glm::uvec4& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w)); }
	template<class Archive> void serialize(Archive& archive, glm::dvec2& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y)); }
	template<class Archive> void serialize(Archive& archive, glm::dvec3& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z)); }
	template<class Archive> void serialize(Archive& archive, glm::dvec4& v) { archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w)); }
	template<class Archive> void serialize(Archive& archive, glm::mat2& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1])); }
	template<class Archive> void serialize(Archive& archive, glm::mat3& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1]), cereal::make_nvp("c2", m[2])); }
	template<class Archive> void serialize(Archive& archive, glm::mat4& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1]), cereal::make_nvp("c2", m[2]), cereal::make_nvp("c3", m[3])); }
	template<class Archive> void serialize(Archive& archive, glm::dmat2& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1])); }
	template<class Archive> void serialize(Archive& archive, glm::dmat3& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1]), cereal::make_nvp("c2", m[2])); }
	template<class Archive> void serialize(Archive& archive, glm::dmat4& m) { archive(cereal::make_nvp("c0", m[0]), cereal::make_nvp("c1", m[1]), cereal::make_nvp("c2", m[2]), cereal::make_nvp("c3", m[3])); }
	template<class Archive> void serialize(Archive& archive, glm::quat& q) { archive(cereal::make_nvp("x", q.x), cereal::make_nvp("y", q.y), cereal::make_nvp("z", q.z), cereal::make_nvp("w", q.w)); }
	template<class Archive> void serialize(Archive& archive, glm::dquat& q) { archive(cereal::make_nvp("x", q.x), cereal::make_nvp("y", q.y), cereal::make_nvp("z", q.z), cereal::make_nvp("w", q.w)); }
}

struct TagComponent {
	std::string tag;

	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag) :
		tag(tag) {}

	template <class Archive>
	void serialize(Archive& ar) { ar(CEREAL_NVP(tag)); }
};

struct TransformComponent {
	glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) :
		translation(translation), rotation(rotation), scale(scale) {}

	template <class Archive>
	void serialize(Archive& ar) { ar(CEREAL_NVP(translation), CEREAL_NVP(rotation), CEREAL_NVP(scale)); }
};

struct MeshComponent {
	std::string filepath;
	// not to serialize
	VertexArray* vao = nullptr;

	MeshComponent() = default;
	MeshComponent(const MeshComponent&);
	MeshComponent(const std::string& filepath);
	~MeshComponent() { /*delete vao;*/ } // TODO: fix component clean up of renderer resources

	void LoadOBJ();

	template <class Archive>
	void serialize(Archive& ar) { ar(CEREAL_NVP(filepath)); }
};

struct MeshRendererComponent {
	enum class Visualization { SolidColor, Normal, UV, Depth, PointLight, HemisphericalLight, };
	static inline const char* visNames[6] = { "SolidColor", "Normal", "UV", "Depth", "PointLight", "HemisphericalLight" }; // for GUI
	struct Depth { 
		float max = 5.0f; 
		float pow = 2.0f; 
		template <class Archive> void serialize(Archive& ar) { ar(CEREAL_NVP(max), CEREAL_NVP(pow)); }
	};

	Visualization visualization = Visualization::Normal;
	Depth depthParams;
	glm::vec4 solidColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(const Visualization& visualization) 
		: visualization(visualization) {}

	template <class Archive>
	void serialize(Archive& ar) { ar(CEREAL_NVP(visualization), CEREAL_NVP(depthParams), CEREAL_NVP(solidColor)); }
};