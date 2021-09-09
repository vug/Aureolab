#include "Renderer/Shader.h"

#include <glad/glad.h>

#include <string>
#include <unordered_map>

class OpenGLShader : public Shader {
public:
	OpenGLShader(const std::string& filepath);
	~OpenGLShader();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual unsigned int GetAttribLocation(const std::string& name) override;

	virtual void UploadUniformInt(const std::string& name, int value) override;
	virtual void UploadUniformFloat(const std::string& name, float value) override;
	virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& values) override;
	virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& values) override;
	virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& values) override;

	virtual void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) override;
	virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
	virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

	virtual void UploadUniformFloats(const std::string& name, const std::vector<float>& values) override;
	virtual void UploadUniformFloat2s(const std::string& name, const std::vector<glm::vec2>& values) override;
	virtual void UploadUniformFloat3s(const std::string& name, const std::vector<glm::vec3>& values) override;
	virtual void UploadUniformFloat4s(const std::string& name, const std::vector<glm::vec4>& values) override;
private:
	unsigned int rendererID;
	std::string ReadFile(const std::string& filepath);
	std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
	void Compile(std::unordered_map<GLenum, std::string>& shaderSources);
};