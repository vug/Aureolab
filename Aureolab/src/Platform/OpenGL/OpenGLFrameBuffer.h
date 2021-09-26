#pragma once

#include "Renderer/FrameBuffer.h"

#include <vector>

class OpenGLFrameBuffer : public FrameBuffer {
public:
	OpenGLFrameBuffer();
	virtual ~OpenGLFrameBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual unsigned int GetColorAttachmentRendererID(unsigned int index) const override;
	virtual void Resize(int width, int height) override;

private:
	unsigned int rendererID = -1;
	std::vector<unsigned int> colorRendererIDs;
	unsigned int depthRendererID;
};