#pragma once

#include "LayerList.h"
#include "Window.h"
#include "GraphicsContext.h"

#include <memory>
#include <string>

class Application : public LayerList {
public:
	Application(const std::string& name);
private:
	void Run();
	Window* window = nullptr;
	GraphicsContext* context = nullptr;
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};