#pragma once
#include <string>

class Layer {
public:
	Layer(const std::string& name) : name(name) {}
	virtual void OnAttach() = 0;
	virtual void OnUpdate(float ts) = 0;
	virtual void OnDetach() = 0;
private:
	std::string name;
};