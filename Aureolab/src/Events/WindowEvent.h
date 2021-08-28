#pragma once
#include "Event.h"

#include <sstream>

class WindowResizeEvent : public Event {
public:
	WindowResizeEvent(unsigned int width, unsigned int height)
		: width(width), height(height) {}

	virtual const char* GetName() const override { return "WindowResizeEvent"; }

	unsigned int GetWidth() const { return width; }
	unsigned int GetHeight() const { return height; }

	std::string ToString() const override {
		std::stringstream ss;
		ss << GetName() << ": " << width << ", " << height;
		return ss.str();
	}
private:
	unsigned int width, height;
};

class WindowCloseEvent : public Event {
public:
	WindowCloseEvent() {};

	virtual const char* GetName() const override { return "WindowCloseEvent"; }

	std::string ToString() const override {
		return GetName();
	}
};

class FrameBufferResizeEvent : public Event {
public:
	FrameBufferResizeEvent(unsigned int width, unsigned int height)
	: width(width), height(height) {}

virtual const char* GetName() const override { return "FrameBufferResizeEvent"; }

unsigned int GetWidth() const { return width; }
unsigned int GetHeight() const { return height; }

std::string ToString() const override {
	std::stringstream ss;
	ss << GetName() << ": " << width << ", " << height;
	return ss.str();
}
private:
	unsigned int width, height;
};