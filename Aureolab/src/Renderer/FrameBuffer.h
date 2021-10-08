#pragma once

#include <glm/glm.hpp>

class FrameBuffer {
public:
	enum class TextureFormat {
		RGBA8, RED_INTEGER,
	};
	static FrameBuffer* Create(int width, int height, TextureFormat textureFormat = TextureFormat::RGBA8);
	virtual ~FrameBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual unsigned int GetColorAttachmentRendererID(unsigned int index = 0) const = 0;
	virtual void Resize(int width, int height) = 0;
	virtual void Clear(int clearValue, unsigned int index = 0) = 0;
	virtual void Clear(glm::vec4 clearColor, unsigned int index = 0) = 0;
	virtual void ReadPixel(int& pixel, int x, int y, unsigned int index = 0) = 0;
	virtual void ReadPixel(glm::vec4& pixel, int x, int y, unsigned int index = 0) = 0;

	//virtual void AddColorAttachment() = 0;
	//virtual void SetDepthAttachment() const = 0;
};