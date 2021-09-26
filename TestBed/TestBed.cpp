#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Layer1.h"
#include "Layer2.h"
#include "Layer3.h"

#include "imgui.h"

#include <vector>

ApplicationConfig TestBedAppConfig = { "AureoLab Test Bed" };

class TestBed : public Application {
public:
	TestBed(std::vector<std::string> args) : Application(TestBedAppConfig) {
		Log::Info("Hi from TestBed! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

		layers = { 
			new Layer1(), 
			new Layer2(),
			new Layer3(),
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

	virtual void OnImGuiRender() override {
		static bool shouldShowDemo = false;

		ImGui::Begin("TestBed");
		ImGui::Text("Welcome to AureoLab TestBed.\nUse [ and ] keys to switch between layers.");
		ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
		ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}

private:
	std::vector<Layer*> layers = {};
	unsigned int activeLayerIndex = 2;
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}