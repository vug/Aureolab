add_library(Aureolab STATIC
	Core/Log.cpp Core/Log.h
    Core/EntryPoint.h 
	Core/Application.cpp Core/Application.h
	Core/Layer.h Core/LayerList.h Core/LayerList.cpp
	Platform/Platform.h
	Core/Window.h Core/Window.cpp
	Platform/WindowsWindow.h  Platform/WindowsWindow.cpp
	Core/GraphicsContext.h Core/GraphicsContext.cpp
    Platform/OpenGL/OpenGLContext.h Platform/OpenGL/OpenGLContext.cpp
	Renderer/VertexBuffer.h Renderer/VertexBuffer.cpp
	Platform/OpenGL/OpenGLVertexBuffer.h 
    Events/Event.h Events/WindowEvent.h Events/KeyEvent.h Events/MouseEvent.h
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
)

target_compile_features(Aureolab PRIVATE cxx_std_17)