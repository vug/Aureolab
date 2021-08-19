#pragma once

#include <string>

class Application {
public:
	Application(const std::string& name);
private:
	void Run();
	std::string name;
	// Applications can be created by client apps but can only be ran from EntryPoint's main
	friend int main(int argc, char* argv[]);
};