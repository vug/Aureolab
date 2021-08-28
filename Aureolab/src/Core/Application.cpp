#include "Application.h"
#include "Events/WindowEvent.h"
#include "Log.h"

#include <string>

Application::Application(const std::string& name) : name(name) { 
    window = Window::Create(name, 1000, 1000);
    window->SetEventCallback(AL_BIND_EVENT_FN(Application::OnEvent));
    context = GraphicsContext::Create(window);

    // Renderer::Initialize
    // TODO: Enable GL debugging, other glEnable (blending, blend function, depth test etc) to application defaults
}

void Application::OnEvent(Event& e) {
    auto dispatcher = EventDispatcher(e);

    dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Application::OnWindowResize));
    dispatcher.Dispatch<WindowCloseEvent>(AL_BIND_EVENT_FN(Application::OnWindowClose));
}

void Application::OnWindowResize(WindowResizeEvent& e) {
    // TODO: Placeholder log for development testing. Will implement properly.
    Log::Info("Application::OnWindowResize received: {}", e.ToString());
}

void Application::OnWindowClose(WindowCloseEvent& e) {
    // TODO: Placeholder log for development testing. Will implement properly.
    Log::Info("Application::OnWindowClose received: {}", e.ToString());
}

void Application::Run() {
	Log::Info("{} app entering main loop...", name);

    float lastUpdateTime = window->GetTime();
    while (window->IsRunning()) {
        float timestep = window->GetTime() - lastUpdateTime;
        lastUpdateTime = window->GetTime();

        for (auto layer : layers) {
            layer->OnUpdate(timestep);
        }
        window->OnUpdate();
        context->OnUpdate();
    }

    for (int i = 0; i < layers.size(); i++) {
        PopLayer();
    }

    window->Shutdown();
    delete window;
}

