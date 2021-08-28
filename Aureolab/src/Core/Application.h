#pragma once

#include "LayerList.h"
#include "Window.h"
#include "GraphicsContext.h"
#include "Events/Event.h"
#include "Events/WindowEvent.h"

#include <memory>
#include <string>

class Application : public LayerList {
public:
	Application(const std::string& name);
	void OnEvent(Event& e);
	void OnWindowClose(WindowCloseEvent& e);
private:
	void Run();
	bool isRunning = true;
	Window* window = nullptr;
	GraphicsContext* context = nullptr;
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};