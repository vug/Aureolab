#pragma once

#include "GLFW/glfw3.h"

#include "Window.h"

#include <string>

// An extension to Application class to keep ImGui preparation logic neatly separate
class ImGuiHelper {
public:
	static void Initialize(Window* window);
	static void BeginFrame();
	static void RenderFrame();
	static void Shutdown();
    // Helper to display a little (?) mark which shows a tooltip when hovered.
    static void InfoMarker(const std::string& desc);
};