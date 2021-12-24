#include "WindowsInput.h"

#include "Core/Window.h"

#include <GLFW/glfw3.h>

#include <cassert>

int Al2GlfwMouseButton(MouseButton button) {
	switch (button) {
	case MouseButton::Left:
		return GLFW_MOUSE_BUTTON_LEFT;
		break;
	case MouseButton::Middle:
		return GLFW_MOUSE_BUTTON_MIDDLE;
		break;
	case MouseButton::Right:
		return GLFW_MOUSE_BUTTON_RIGHT;
		break;
	default:
		assert(false); // shouldn't reach here
		return -1;
	}
}

bool WindowsInput::IsKeyPressed(int key) {
	GLFWwindow* glfwWindow = (GLFWwindow*)window->GetNativeWindow();
	int state = glfwGetKey(glfwWindow, key);
	return state == GLFW_PRESS;
}

bool WindowsInput::IsMouseButtonPressed(MouseButton button) {
	GLFWwindow*  glfwWindow = (GLFWwindow*)window->GetNativeWindow();
	int state = glfwGetMouseButton(glfwWindow, Al2GlfwMouseButton(button));
	return state == GLFW_PRESS;
}

glm::vec2 WindowsInput::GetMouseCursorPosition() {
	GLFWwindow* glfwWindow = (GLFWwindow*)window->GetNativeWindow();
	double xpos, ypos;
	glfwGetCursorPos(glfwWindow, &xpos, &ypos);
	return glm::vec2((float)xpos, (float)ypos);
}
