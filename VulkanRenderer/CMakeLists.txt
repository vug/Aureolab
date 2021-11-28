add_executable(VulkanRenderer 
	main.cpp
 "VulkanRenderer.h" "VulkanRenderer.cpp" 
 "VulkanWindow.h" "VulkanWindow.cpp" 
 "IResizable.h" 
 "VulkanContext.h" "VulkanContext.cpp" 
 "VulkanDestroyer.h" 
 "Mesh.h" "Mesh.cpp"
 "Types.h")

target_link_libraries(
	VulkanRenderer PRIVATE 
	Aureolab
)

#add_custom_target(compile_vulkan_study_shaders
#    COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/compile.bat
#)
#add_dependencies(VulkanStudy compile_vulkan_study_shaders)

add_custom_target(copy_vulkan_renderer_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(VulkanRenderer copy_vulkan_renderer_assets)

target_compile_features(VulkanRenderer PRIVATE cxx_std_20)