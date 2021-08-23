#pragma once
#include "Window.h"

enum class GraphicsAPI {
	OPENGL,
	// DIRECTX, VULKAN
};

class GraphicsContext {
public:
	static GraphicsContext* Create(Window* window); // factory method design pattern
	static GraphicsAPI graphicsAPI;

	virtual void OnUpdate() = 0;

	virtual void SwapBuffers() = 0;
	virtual void SetVSync(bool toggle) = 0;
};