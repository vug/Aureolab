#pragma once

#include "Core/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class WindowsWindow : public Window {
public:
	WindowsWindow(const std::string& name, int width, int height);
	virtual void OnUpdate() override;
	virtual bool IsRunning() override;
	virtual void Shutdown() override;

	virtual int GetWidth() override;
	virtual int GetHeight() override;
private:
	GLFWwindow* window;
};