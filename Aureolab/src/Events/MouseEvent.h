#pragma once

#include <sstream>
#include <string>

class MouseMovedEvent : public Event {
public:
	MouseMovedEvent(const float x, const float y)
		: mouseX(x), mouseY(y) {}

	virtual const char* GetName() const override { return "MoveMovedEvent"; }

	float GetX() const { return mouseX; }
	float GetY() const { return mouseY; }

	std::string ToString() const override {
		std::stringstream ss;
		ss << GetName() << ": " << mouseX << ", " << mouseY;
		return ss.str();
	}
private:
	float mouseX, mouseY;
};

class MouseScrolledEvent : public Event {
public:
	MouseScrolledEvent(const float xOffset, const float yOffset)
		: xOffset(xOffset), yOffset(yOffset) {}

	virtual const char* GetName() const override { return "MouseScrollEvent"; }

	float GetXOffset() const { return xOffset; }
	float GetYOffset() const { return yOffset; }

	std::string ToString() const override {
		std::stringstream ss;
		ss << GetName() << ": " << xOffset << ", " << yOffset;
		return ss.str();
	}
private:
	float xOffset, yOffset;
};

class MouseButtonEvent : public Event {
public:
	int GetMouseButton() const { return button; }
protected:
	MouseButtonEvent(const int button)
		: button(button) {}

	int button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
	MouseButtonPressedEvent(const int button)
		: MouseButtonEvent(button) {}

	virtual const char* GetName() const override { return "MouseButtonPressedEvent"; }

	std::string ToString() const override {
		std::stringstream ss;
		ss << GetName() << ": " << button;
		return ss.str();
	}
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
	MouseButtonReleasedEvent(const int button1)
		: MouseButtonEvent(button1) {}

	virtual const char* GetName() const override { return "MouseButtonReleasedEvent"; }

	std::string ToString() const override {
		std::stringstream ss;
		ss << GetName() << ": " << button;
		return ss.str();
	}
};