#pragma once
#include "Window.h"

class GraphicsContext {
public:
	enum class API {
		OPENGL,
		// DIRECTX, VULKAN
	};
	static GraphicsContext* Create(Window* window); // factory method design pattern
	static API graphicsAPI;

	virtual void OnUpdate() = 0;

	virtual void SwapBuffers() = 0;
	virtual void SetViewportSize(unsigned int width, unsigned int height) = 0;
	virtual void SetVSync(bool toggle) = 0;
};