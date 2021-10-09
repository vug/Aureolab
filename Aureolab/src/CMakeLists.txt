add_library(Aureolab STATIC
	Core/Log.cpp Core/Log.h
    Core/EntryPoint.h 
	Core/Application.cpp Core/Application.h
	Core/Layer.h Core/LayerList.h Core/LayerList.cpp
	Events/Event.h Events/WindowEvent.h Events/KeyEvent.h Events/MouseEvent.h
	Platform/Platform.h
	Core/Window.h Core/Window.cpp
	Platform/Windows/WindowsWindow.h Platform/Windows/WindowsWindow.cpp
	Core/GraphicsContext.h Core/GraphicsContext.cpp
    Platform/OpenGL/OpenGLContext.h Platform/OpenGL/OpenGLContext.cpp
	Renderer/VertexSpecification.h Platform/OpenGL/OpenGLVertexSpecification.h Platform/OpenGL/OpenGLVertexSpecification.cpp
	Renderer/VertexBuffer.h Renderer/VertexBuffer.cpp
	Platform/OpenGL/OpenGLVertexBuffer.h Platform/OpenGL/OpenGLVertexBuffer.cpp
	Renderer/IndexBuffer.h Renderer/IndexBuffer.cpp
	Platform/OpenGL/OpenGLIndexBuffer.h Platform/OpenGL/OpenGLIndexBuffer.cpp
	Renderer/VertexArray.h Renderer/VertexArray.cpp
	Platform/OpenGL/OpenGLVertexArray.h Platform/OpenGL/OpenGLVertexArray.cpp
	Renderer/FrameBuffer.h Renderer/FrameBuffer.cpp
	Platform/OpenGL/OpenGLFrameBuffer.h Platform/OpenGL/OpenGLFrameBuffer.cpp
    Renderer/Shader.h Renderer/Shader.cpp
	Platform/OpenGL/OpenGLShader.h Platform/OpenGL/OpenGLShader.cpp
    Renderer/GraphicsAPI.h Renderer/GraphicsAPI.cpp
	Platform/OpenGL/OpenGLGraphicsAPI.h Platform/OpenGL/OpenGLGraphicsAPI.cpp
	Renderer/Renderer.h Renderer/Renderer.cpp
    Core/ImGuiHelper.h Core/ImGuiHelper.cpp
	Core/Math.h Core/Math.cpp
	Core/Input.h Core/Input.cpp
    Platform/Windows/WindowsInput.h Platform/Windows/WindowsInput.cpp
	Modeling/Modeling.h Modeling/Modeling.cpp
    Renderer/EditorCamera.h Renderer/EditorCamera.cpp
	Scene/Components.h Scene/Components.cpp
	Scene/Scene.h Scene/Scene.cpp
    Platform/Windows/WindowsPlatformUtils.h Platform/Windows/WindowsPlatformUtils.cpp
)

# If a debug build configuration define AL_DEBUG processor macro
add_compile_definitions("$<$<CONFIG:Debug>:AL_DEBUG>")
add_compile_definitions("$<$<CONFIG:RelWithDebInfo>:AL_DEBUG>")

# Make Aureolab/src header files to be included via relative path from src/ folder, e.g. "Core/Application.h" or "Platform/Platform.h"
target_include_directories(Aureolab PUBLIC .)

target_link_libraries(
	Aureolab PUBLIC 
	spdlog
	opengl32  # assuming this is system-wide available
	glad
	glfw
	glm
	imgui
	tinyobjloader
	EnTT::EnTT
	cereal
	imguizmo
)

target_compile_features(Aureolab PRIVATE cxx_std_17)