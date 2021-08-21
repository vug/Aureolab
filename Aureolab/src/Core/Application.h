#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <string>

class Application {
public:
	Application(const std::string& name);
	virtual void OnStart() = 0;
	virtual void OnUpdate(float timestep) = 0;
	virtual void OnEnd() = 0;
	virtual void OnKeyPress(int key, int scancode, int action, int mods) = 0;

	void Close();
	GLFWwindow* GetWindow() { return window; }
private:
	void Run();
	GLFWwindow* window = nullptr;
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};