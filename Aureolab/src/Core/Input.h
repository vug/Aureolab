#pragma once

#include "Core/Window.h"

#include <glm/glm.hpp>

enum class MouseButton {
    Left, Middle, Right,
};

class Input {
public:
    static void Initialize(Window* window);
    static Input* Get();

	virtual bool IsMouseButtonPressed(MouseButton button) = 0;
    virtual glm::vec2 GetMouseCursorPosition() = 0;

protected:
	static Input* instance;
    static Window* window;
};