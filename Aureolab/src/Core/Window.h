#pragma once

#include <string>

class Window {
public:
	static Window* Create(const std::string& name, int width, int height); // factory method design pattern

	virtual void OnUpdate() = 0;
	virtual bool IsRunning() = 0;
	virtual void Shutdown() = 0;
	
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	// needs to be casted to concrete Window after acquired
	virtual void* GetNativeWindow() = 0;
};