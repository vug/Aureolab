#include "Application.h"
#include "Events/WindowEvent.h"
#include "Log.h"

#include <string>

Application::Application(const std::string& name) : name(name) { 
    window = Window::Create(name, 1000, 1000);
    window->SetEventCallback(AL_BIND_EVENT_FN(Application::OnEventApplication));
    context = GraphicsContext::Create(window);

    // Renderer::Initialize
}

void Application::OnEventApplication(Event& ev) {
    auto dispatcher = EventDispatcher(ev);
    dispatcher.Dispatch<WindowCloseEvent>(AL_BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<FrameBufferResizeEvent>(AL_BIND_EVENT_FN(Application::OnFrameBufferResized));

    OnEvent(ev); // to client app
    for (int i = 0; i < layers.size(); i++) {
        auto layer = layers[i];
        if (layer->GetIsRunning()) 
            layer->OnEvent(ev); // to client app's layers
    }
}

void Application::OnWindowClose(WindowCloseEvent& e) {
    isRunning = false;
}

void Application::OnFrameBufferResized(FrameBufferResizeEvent& ev) {
    context->SetViewportSize(ev.GetWidth(), ev.GetHeight());
}

void Application::Run() {
	Log::Info("{} app entering main loop...", name);

    float lastUpdateTime = window->GetTime();
    while (isRunning) {
        float timestep = window->GetTime() - lastUpdateTime;
        lastUpdateTime = window->GetTime();

        for (auto layer : layers) {
            if (layer->GetIsRunning())
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

