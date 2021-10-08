#pragma once

#include "Renderer/FrameBuffer.h"

#include <vector>

class OpenGLFrameBuffer : public FrameBuffer {
public:
	OpenGLFrameBuffer(int width, int height, TextureFormat textureFormat);
	virtual ~OpenGLFrameBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual unsigned int GetColorAttachmentRendererID(unsigned int index) const override;
	virtual void Resize(int width, int height) override;
	virtual void Clear(int clearValue, unsigned int index) override;
	virtual void Clear(glm::vec4 clearColor, unsigned int index = 0) override;
	virtual void ReadPixel(int& pixel, int x, int y, unsigned int index = 0) override;
	virtual void ReadPixel(glm::vec4& pixel, int x, int y, unsigned int index = 0) override;

private:
	unsigned int rendererID = -1;
	std::vector<unsigned int> colorRendererIDs;
	unsigned int depthRendererID;
	TextureFormat textureFormat;
};