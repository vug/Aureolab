#include "Application.h"
#include "Events/WindowEvent.h"
#include "Log.h"
#include "ImGuiHelper.h"
#include "Input.h"

#include <string>

Application::Application(const ApplicationConfig& config) : name(config.name) {
    window = Window::Create(name, config.windowWidth, config.windowHeight);
    window->SetEventCallback(AL_BIND_EVENT_FN(Application::OnEventApplication));

    GraphicsContext::Initialize(window);
    ImGuiHelper::Initialize(window);
    Input::Initialize(window);
}

void Application::OnEventApplication(Event& ev) {
    auto dispatcher = EventDispatcher(ev);
    dispatcher.Dispatch<WindowCloseEvent>(AL_BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<FrameBufferResizeEvent>(AL_BIND_EVENT_FN(Application::OnFrameBufferResized));

    OnEvent(ev); // to client app
    for (int i = 0; i < layers.size(); i++) {
        auto layer = layers[i];
        if (layer->GetIsRunning()) {
            layer->OnEvent(ev); // to client app's layers
        }
    }
}

void Application::OnWindowClose(WindowCloseEvent& e) {
    isRunning = false;
}

void Application::OnFrameBufferResized(FrameBufferResizeEvent& ev) {
    GraphicsContext::Get()->SetViewportSize(ev.GetWidth(), ev.GetHeight());
}

void Application::Run() {
	Log::Info("{} app entering main loop...", name);

    float lastUpdateTime = window->GetTime();
    while (isRunning) {
        float timestep = window->GetTime() - lastUpdateTime;
        lastUpdateTime = window->GetTime();

        ImGuiHelper::BeginFrame();
        OnImGuiRender();
        for (auto layer : layers) {
            if (layer->GetIsRunning()) {
                layer->OnImGuiRender();
                layer->OnUpdate(timestep);
            }
        }
        ImGuiHelper::RenderFrame();
        window->OnUpdate();
        GraphicsContext::Get()->OnUpdate();
    }

    for (int i = 0; i < layers.size(); i++) {
        PopLayer();
    }

    ImGuiHelper::Shutdown();
    window->Shutdown();
    delete window;
}

