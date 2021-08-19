#include "Core/EntryPoint.h"
#include "Core/Log.h"

#include <iostream>

class TestBed : public Application {
public:
	TestBed(std::vector<std::string> args) : Application("AureLab Test Bed") {
		Log::Info("Hi from TestBed! argc: {}, argv[0]: {}", args.size(), args[0]);

		Log::Critical("This is so critical!");
		Log::Error("Halt! Ein Error!");
		Log::Warning("I'm warning you.");
		Log::Info("Hello, Aureolab!");
		Log::Debug("There are {} many bugs in this software", 42);
		Log::Trace("Tracing...");
	}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}