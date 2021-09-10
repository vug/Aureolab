#pragma once
#include "Events/Event.h"
#include <string>

class Layer {
public:
	Layer(const std::string& name) : name(name) {}
	const bool GetIsRunning() const { return isRunning; }
	void TogglePause() { isRunning = !isRunning; }
	void Pause() { isRunning = false; }
	void Play() { isRunning = true; }

	virtual void OnEvent(Event& ev) = 0;
	virtual void OnAttach() = 0;
	virtual void OnUpdate(float ts) = 0;
	virtual void OnDetach() = 0;
private:
	std::string name;
	bool isRunning = true;
};