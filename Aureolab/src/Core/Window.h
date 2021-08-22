#pragma once

#include <string>

class Window {
public:
	static Window* CreateAndInitialize(const std::string& name, int width, int height);

	virtual void Initialize(const std::string& name, int width, int height) = 0;
	virtual void OnUpdate() = 0;
	virtual bool IsRunning() = 0;
	virtual void Shutdown() = 0;
	
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
};