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

		layers = { 
			new Layer1(), 
			new Layer2(),
		};
		PlaySingleLayer(activeLayerIndex);

		for (Layer* layer : layers) {
			PushLayer(layer);
		}
	}

private:
	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(TestBed::OnKeyPressed));
	}

	void OnKeyPressed(KeyPressedEvent& ev) {
		switch (ev.GetKeyCode()) {
		case 93:
			activeLayerIndex++;
			break;
		case 91:
			activeLayerIndex--;
		}
		activeLayerIndex = activeLayerIndex % layers.size();
		PlaySingleLayer(activeLayerIndex);
	}

	void PlaySingleLayer(unsigned int ix) {
		for (unsigned int j = 0; j < layers.size(); j++) {
			layers[j]->Pause();
		}
		layers[ix]->Play();
	}

private:
	std::vector<Layer*> layers = {};
	unsigned int activeLayerIndex = 1;
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}