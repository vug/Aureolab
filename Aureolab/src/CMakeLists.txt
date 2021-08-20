add_library(Aureolab STATIC
	 
	Core/Log.cpp Core/Log.h
    Core/EntryPoint.h 
	Core/Application.cpp Core/Application.h
)

target_link_libraries(
	Aureolab PUBLIC 
	spdlog
	opengl32  # assuming this is system-wide available
	glfw
)

target_compile_features(Aureolab PRIVATE cxx_std_17)