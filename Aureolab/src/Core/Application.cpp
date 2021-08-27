#include "Application.h"
#include "Log.h"

#include <string>

Application::Application(const std::string& name) : name(name) { 
    window = Window::Create(name, 1000, 1000);
    context = GraphicsContext::Create(window);

    // Renderer::Initialize
    // TODO: Enable GL debugging, other glEnable (blending, blend function, depth test etc) to application defaults
}

void Application::Run() {
	Log::Info("{} app entering main loop...", name);

    float lastUpdateTime = window->GetTime();
    while (window->IsRunning()) {
        float timestep = window->GetTime() - lastUpdateTime;
        lastUpdateTime - timestep;

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
