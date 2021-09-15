#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"

#include "ShaderLayer.h"

class ShaderBoy : public Application {
public:
	ShaderBoy(std::vector<std::string> args) : Application("AureLab ShaderBoy") {
		Log::Info("Hi from ShaderBoy! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

		PushLayer(new ShaderLayer());
	}

private:
	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		//dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(TestBed::OnKeyPressed));
	}

	virtual void OnImGuiRender() override {}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new ShaderBoy(args);
}