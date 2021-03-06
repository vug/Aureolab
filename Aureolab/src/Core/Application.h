#pragma once

#include "LayerList.h"
#include "Window.h"
#include "GraphicsContext.h"
#include "Events/Event.h"
#include "Events/WindowEvent.h"

#include <memory>
#include <string>

struct ApplicationConfig {
	std::string name;
	int windowWidth = 1000;
	int windowHeight = 1000;
};

class Application : public LayerList {
public:
	Application(const ApplicationConfig& config);
protected:
	virtual void OnEvent(Event& ev) = 0;
private:
	void Run();
	void OnEventApplication(Event& ev);
	void OnWindowClose(WindowCloseEvent& ev);
	void OnFrameBufferResized(FrameBufferResizeEvent& ev);

	virtual void OnImGuiRender() = 0;
private:
	bool isRunning = true;
	Window* window = nullptr;
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};