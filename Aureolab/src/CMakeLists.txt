add_executable(Aureolab 
	Aureolab.cpp 
	Core/Log.cpp Core/Log.h
)

target_link_libraries(
	Aureolab PRIVATE 
	spdlog
)

target_compile_features(Aureolab PRIVATE cxx_std_17)