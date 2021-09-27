#pragma once

#include "Core/Input.h"

class WindowsInput : public Input {
public:
	WindowsInput() = default;

	virtual bool IsKeyPressed(int key) override;
	virtual bool IsMouseButtonPressed(MouseButton button) override;
	virtual glm::vec2 GetMouseCursorPosition() override;
};