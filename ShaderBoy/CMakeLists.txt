add_executable(ShaderBoy 
	ShaderLayer.h ShaderBoy.cpp
	FileWatcher.h
)

target_link_libraries(
	ShaderBoy PRIVATE 
	Aureolab
)

target_include_directories(ShaderBoy PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)


add_custom_target(copy_shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

add_dependencies(ShaderBoy copy_shaders)

target_compile_features(ShaderBoy PRIVATE cxx_std_17)