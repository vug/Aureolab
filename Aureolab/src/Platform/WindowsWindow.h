#pragma once

#include "Core/Window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class WindowsWindow : public Window {
public:
	WindowsWindow(const std::string& name, int width, int height);
	virtual void SetEventCallback(const EventCallbackFn callback) override { userPointer.eventCallback = callback; };

	virtual void OnUpdate() override;
	virtual void Shutdown() override;

	virtual int GetWidth() override;
	virtual int GetHeight() override;

	virtual float GetTime() override;
	
	// needs to be casted after acquired
	virtual void* GetNativeWindow() override { return window; }
private:
	GLFWwindow* window;

	struct UserPointer {
		EventCallbackFn eventCallback;
		void Dispatch(Event& ev) { if (eventCallback != nullptr) eventCallback(ev); }
	} userPointer;
};