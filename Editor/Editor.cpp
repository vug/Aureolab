#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Events/KeyEvent.h"
#include "EditorLayer.h"

#include "imgui.h"

#include <vector>

class Editor : public Application {
public:
	Editor(std::vector<std::string> args) : Application("AureoLab Editor") {
		Log::Info("Hi from Editor! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

		PushLayer(new EditorLayer());
	}

private:
	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(Editor::OnKeyPressed));
	}

	void OnKeyPressed(KeyPressedEvent& ev) {
	}

	virtual void OnImGuiRender() override {
		static bool shouldShowDemo = false;

		ImGui::Begin("Editor");
		ImGui::Text("Welcome to AureoLab Editor.");
		ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
		ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new Editor(args);
}