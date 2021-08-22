#include "Window.h"
#include "Platform/Platform.h"
#include "Platform/WindowsWindow.h"

#include <cassert>

Window* Window::CreateAndInitialize(const std::string& name, int width, int height) {
	Window* w = nullptr;
	switch (PlatformUtils::GetPlatform()) {
	case Platform::WINDOWS:
		w = new WindowsWindow();
		break;
	default:
		assert("Concrete window for this platform is not implemented yet.");
	}

	w->Initialize(name, width, height);
	return w;
}