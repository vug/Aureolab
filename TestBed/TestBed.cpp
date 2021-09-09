#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Layer1.h"
#include "Layer2.h"

class TestBed : public Application {
public:
	TestBed(std::vector<std::string> args) : Application("AureLab Test Bed") {
		Log::Info("Hi from TestBed! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

        PushLayer(new Layer1());
		PushLayer(new Layer2());
	}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}