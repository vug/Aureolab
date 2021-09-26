#pragma once

class FrameBuffer {
public:
	static FrameBuffer* Create(int width, int height);
	virtual ~FrameBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual unsigned int GetColorAttachmentRendererID(unsigned int index = 0) const = 0;
	virtual void Resize(int width, int height) = 0;

	//virtual void AddColorAttachment() = 0;
	//virtual void SetDepthAttachment() const = 0;
};