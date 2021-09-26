#pragma once

#include "Renderer/FrameBuffer.h"

class OpenGLFrameBuffer : public FrameBuffer {
public:
	OpenGLFrameBuffer();
	virtual ~OpenGLFrameBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

private:
	unsigned int rendererID = -1;
};