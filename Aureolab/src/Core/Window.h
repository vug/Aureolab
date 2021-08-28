#pragma once
#include "Events/Event.h"

#include <functional>
#include <string>

class Window {
public:
	static Window* Create(const std::string& name, int width, int height); // factory method design pattern
	using EventCallbackFn = std::function<void(Event&)>;
	virtual void SetEventCallback(const EventCallbackFn callback) = 0;

	virtual void OnUpdate() = 0;
	virtual bool IsRunning() = 0;
	virtual void Shutdown() = 0;
	
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual float GetTime() = 0;

	// needs to be casted to concrete Window after acquired
	virtual void* GetNativeWindow() = 0;
};