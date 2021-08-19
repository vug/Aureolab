#include "Application.h"

#include <string>
#include <vector>

extern Application* CreateApplication(std::vector<std::string> cliArgs);

int main(int argc, char* argv[]) {
	std::vector<std::string> cliArgs;
	for (int i = 0; i < argc; i++) {
		cliArgs.push_back(argv[i]);
	}

	auto app = CreateApplication(cliArgs);

	app->Run();

	delete app;

	return 0;
}