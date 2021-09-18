#include "Input.h"

#include "Platform/Platform.h"
#include "Platform/Windows/WindowsInput.h"

#include <cassert>

void Input::Initialize(Window* window) {
	Input::window = window;
}

Input* Input::Get() {
	if (instance == nullptr) {
		switch (PlatformUtils::GetPlatform()) {
		case Platform::WINDOWS:
			instance = new WindowsInput();
			break;
		default:
			assert("Concrete inputs for this platform has not been implemented yet.");
		}
	}
	return instance;
}

Input* Input::instance = nullptr;
Window* Input::window = nullptr;