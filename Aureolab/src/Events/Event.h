#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

//#define AL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
// black magic that replaces above bind with lambda function.
#define AL_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

class Event {
public:
	virtual ~Event() = default;

	// bool handled = false; // TODO: Implement for stopping spread of an event. Say, after top layer handles the event, lower layers won't receive it.

	virtual const char* GetName() const = 0;
	virtual std::string ToString() const { return GetName(); }
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
	return os << e.ToString();
}


class EventDispatcher {
public:
	EventDispatcher(Event& event)
		: event(event) {}

	template<typename TEvent> // A Concrete Event class
	bool Dispatch(std::function<void(TEvent&)> func) {
		static_assert(std::is_base_of_v<Event, TEvent>, "Given type should be derived from Event class");

		// Check whether incoming concrete event type is the target type of this Dispatch call
		if (typeid(TEvent) == typeid(event)) { // Reference to event cannot be dynamic_cast'ed safely
			func(static_cast<TEvent&>(event));
			return true;
		}
		else { // Types mismatch, not dispatching/sending.
			return false;
		}
	}
private:
	Event& event;
};