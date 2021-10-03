#pragma once
#include "Event.h"

#include <string>
#include <sstream>

class KeyEvent : public Event {
public:
	int GetKeyCode() const { return keyCode; }
protected:
	KeyEvent(int keyCode) 
		: keyCode(keyCode) {}
	int keyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
	KeyPressedEvent(int keyCode, bool isRepeat)
		: KeyEvent(keyCode), isRepeat(isRepeat) {}

	virtual const char* GetName() const override { return "KeyPressedEvent"; }
	
	bool GetIsRepeat() const { return isRepeat; }

	std::string ToString() {
		std::stringstream ss;
		ss << GetName() << ": " << keyCode << " is repeated: " << isRepeat;
		return ss.str();
	}
private:
	bool isRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
	KeyReleasedEvent(int keyCode) 
		: KeyEvent(keyCode) {}

	virtual const char* GetName() const override { return "KeyReleasedEvent"; }

	std::string ToString() {
		std::stringstream ss;
		ss << GetName() << ": " << GetKeyCode();
		return ss.str();
	}
};