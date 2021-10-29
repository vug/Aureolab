add_executable(VulkanStudy 
	main.cpp
)

target_link_libraries(
	VulkanStudy PRIVATE 
	Aureolab
)

add_custom_target(compile_vulkan_study_shaders
    COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/compile.bat
)
add_dependencies(VulkanStudy compile_vulkan_study_shaders)

add_custom_target(copy_vulkan_study_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(VulkanStudy copy_vulkan_study_assets)

target_compile_features(VulkanStudy PRIVATE cxx_std_20)