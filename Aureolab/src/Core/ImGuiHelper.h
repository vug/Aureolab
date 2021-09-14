#pragma once

#include "GLFW/glfw3.h"

#include "Window.h"

// An extension to Application class to keep ImGui preparation logic neatly separate
class ImGuiHelper {
public:
	static void Initialize(Window* window);
	static void BeginFrame();
	static void RenderFrame();
	static void Shutdown();
};