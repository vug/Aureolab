#include "Window.h"
#include "Platform/Platform.h"
#include "Platform/WindowsWindow.h"

#include <cassert>

Window* Window::Create(const std::string& name, int width, int height) {
	Window* w = nullptr;
	switch (PlatformUtils::GetPlatform()) {
	case Platform::WINDOWS:
		w = new WindowsWindow(name, width, height);
		break;
	default:
		assert("Concrete window for this platform has not been implemented yet.");
	}
	return w;
}