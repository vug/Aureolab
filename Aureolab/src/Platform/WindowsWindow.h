#pragma once

#include "Core/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class WindowsWindow : public Window {
public:
	virtual void Initialize(const std::string& name, int width, int height) override;
	virtual void OnUpdate() override;
	virtual bool IsRunning() override;
	virtual void Shutdown() override;

	virtual int GetWidth() override;
	virtual int GetHeight() override;

	void OnKeyPress(int key, int scancode, int action, int mods);
private:
	GLFWwindow* window;
};