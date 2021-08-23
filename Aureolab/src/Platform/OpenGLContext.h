#pragma once
#include "Core/Window.h"
#include "Core/GraphicsContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using GLFWcontext = GLFWwindow;

class OpenGLContext : public GraphicsContext {
public:
	OpenGLContext(Window* window);

	virtual void OnUpdate() override;

	virtual void SetVSync(bool toggle) override;
private:
	virtual void SwapBuffers() override;
	GLFWcontext* context = nullptr;
};