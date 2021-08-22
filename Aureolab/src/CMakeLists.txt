add_library(Aureolab STATIC
	Core/Log.cpp Core/Log.h
    Core/EntryPoint.h 
	Core/Application.cpp Core/Application.h
	Core/Layer.h Core/LayerList.h Core/LayerList.cpp
	Core/Window.h Core/Window.cpp
	Platform/Platform.h
	Platform/WindowsWindow.h  Platform/WindowsWindow.cpp
)

target_include_directories(Aureolab PUBLIC
	.
)

target_link_libraries(
	Aureolab PUBLIC 
	spdlog
	opengl32  # assuming this is system-wide available
	glad
	glfw
)

target_compile_features(Aureolab PRIVATE cxx_std_17)