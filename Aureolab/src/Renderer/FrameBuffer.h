#pragma once

class FrameBuffer {
public:
	static FrameBuffer* Create();
	virtual ~FrameBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	//virtual void AddColorAttachment() = 0;
	//virtual void SetDepthAttachment() const = 0;
};