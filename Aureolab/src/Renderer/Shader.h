#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

class Shader {
public:
	static Shader* Create(const std::string& filepath);

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual unsigned int GetAttribLocation(const std::string& name) = 0;

	virtual void UploadUniformInt(const std::string& name, int value) = 0;
	virtual void UploadUniformFloat(const std::string& name, float value) = 0;
	virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& values) = 0;
	virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& values) = 0;
	virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& values) = 0;

	virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) = 0;
	virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
	virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

	virtual void UploadUniformFloats(const std::string& name, const std::vector<float>& values) = 0;
	virtual void UploadUniformFloat2s(const std::string& name, const std::vector<glm::vec2>& values) = 0;
	virtual void UploadUniformFloat3s(const std::string& name, const std::vector<glm::vec3>& values) = 0;
	virtual void UploadUniformFloat4s(const std::string& name, const std::vector<glm::vec4>& values) = 0;
};