#include "Application.h"
#include "Log.h"

#include <string>

Application::Application(const std::string& name) : name(name) { }

void Application::Run() {
	Log::Info("{} is running", name);
}