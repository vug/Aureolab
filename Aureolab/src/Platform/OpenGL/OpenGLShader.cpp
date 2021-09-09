#include "OpenGLShader.h"

#include "Core/Log.h"

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cassert>
#include <fstream>

OpenGLShader::OpenGLShader(const std::string& filepath) {
	std::string source = ReadFile(filepath);
	auto shaderSources = PreProcess(source);
	Compile(shaderSources);
}

OpenGLShader::~OpenGLShader() {
	glDeleteProgram(rendererID);
}

void OpenGLShader::Bind() const {
	glUseProgram(rendererID);
}

void OpenGLShader::Unbind() const {
	glUseProgram(0);
}

unsigned int OpenGLShader::GetAttribLocation(const std::string& name) {
	return glGetAttribLocation(rendererID, name.c_str());
}

void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform1i(location, value);
}

void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform1f(location, value);
}

void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform2f(location, values.x, values.y);
}

void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform3f(location, values.x, values.y, values.z);
}

void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform4f(location, values.x, values.y, values.z, values.w);
}

void OpenGLShader::UploadUniformMat2(const std::string& name, const glm::mat2& matrix) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::UploadUniformFloats(const std::string& name, const std::vector<float>& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform1fv(location, (GLsizei)values.size(), values.data());
}

void OpenGLShader::UploadUniformFloat2s(const std::string& name, const std::vector<glm::vec2>& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform2fv(location, (GLsizei)values.size() * 2, glm::value_ptr(values.data()[0]));
}

void OpenGLShader::UploadUniformFloat3s(const std::string& name, const std::vector<glm::vec3>& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform3fv(location, (GLsizei)values.size() * 3, glm::value_ptr(values.data()[0]));
}

void OpenGLShader::UploadUniformFloat4s(const std::string& name, const std::vector<glm::vec4>& values) {
	GLint location = glGetUniformLocation(rendererID, name.c_str());
	glUniform4fv(location, (GLsizei)values.size() * 4, glm::value_ptr(values.data()[0]));
}

std::string OpenGLShader::ReadFile(const std::string& filepath) {
	std::ifstream input_file(filepath);
	assert(input_file.is_open()); // file should be openable
	return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

static GLenum ShaderTypeFromString(const std::string& type) {
	if (type == "vertex")
		return GL_VERTEX_SHADER;
	if (type == "fragment" || type == "pixel")
		return GL_FRAGMENT_SHADER;
	if (type == "geometry")
		return GL_GEOMETRY_SHADER;

	assert(false); // Unknown shader type
	return 0;
}

std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) {
	std::unordered_map<GLenum, std::string> shaderSources;

	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0);
	while (pos != std::string::npos) {
		size_t eol = source.find_first_of("\r\n", pos);
		assert(eol != std::string::npos); // Syntax Error
		size_t begin = pos + typeTokenLength + 1;
		std::string type = source.substr(begin, eol - begin);
		assert(ShaderTypeFromString(type)); // Invalid shader type specification

		size_t nextLinePos = source.find_first_not_of("\r\n", eol);
		pos = source.find(typeToken, nextLinePos);
		shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? 
			source.substr(nextLinePos) : 
			source.substr(nextLinePos, pos - nextLinePos);
	}

	return shaderSources;
}

void OpenGLShader::Compile(std::unordered_map<GLenum, std::string>& shaderSources) {
	GLuint program = glCreateProgram();
	assert(shaderSources.size() <= 3); // We only support a geometry, a vertex and a fragment shader for now.
	std::array<GLenum, 3> glShaderIDs;
	int glShaderIdIndex = 0;
	for (auto& kv : shaderSources) {
		GLenum type = kv.first;
		const std::string& source = kv.second;

		// Taken from https://www.khronos.org/opengl/wiki/Shader_Compilation#Example and modified
		GLuint shader = glCreateShader(type);

		const GLchar* sourceCStr = (const GLchar*)source.c_str();
		glShaderSource(shader, 1, &sourceCStr, 0);

		glCompileShader(shader);

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(shader);

			Log::Error("Shader compilation failure.\n{}", infoLog.data());
			return;
		}

		glAttachShader(program, shader);
		glShaderIDs[glShaderIdIndex++] = shader;
	}

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		glDeleteProgram(program);
		for (auto id : glShaderIDs)
			glDeleteShader(id);

		Log::Error("Shader link failure.\n{}", infoLog.data());
		return;
	}

	// Delete only existing shaders
	for (int ix = 0; ix < glShaderIdIndex; ix++) {
		int id = glShaderIDs[ix];
		glDetachShader(program, id);
		glDeleteShader(id);
	}

	rendererID = program;
}
