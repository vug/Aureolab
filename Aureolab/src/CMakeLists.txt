add_executable (Aureolab Aureolab.cpp)

target_link_libraries(
	Aureolab PRIVATE 
	spdlog
)

target_compile_features(Aureolab PRIVATE cxx_std_17)