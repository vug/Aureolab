#pragma once

#include "LayerList.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <string>

class Application : public LayerList {
public:
	Application(const std::string& name);
	GLFWwindow* GetWindow() { return window; }
	void OnKeyPress(int key, int scancode, int action, int mods);
	void Close();
private:
	void Run();
	GLFWwindow* window = nullptr;
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};