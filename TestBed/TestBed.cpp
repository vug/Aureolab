#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Layer1.h"
#include "Layer2.h"

#include <vector>

class TestBed : public Application {
public:
	TestBed(std::vector<std::string> args) : Application("AureLab Test Bed") {
		Log::Info("Hi from TestBed! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

		std::vector<Layer*> layers = { 
			//new Layer1(), 
			new Layer2(),
		};

		for (Layer* layer : layers) {
			PushLayer(layer);
		}
	}
	virtual void OnEvent(Event& ev) override {
		Log::Info("TestBed received {}", ev.ToString());
	}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}