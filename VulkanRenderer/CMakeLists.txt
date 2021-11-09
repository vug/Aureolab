add_executable(VulkanRenderer 
	main.cpp
 "VulkanRenderer.h" "VulkanRenderer.cpp" "Window.h" "Window.cpp" "IResizable.h")

target_link_libraries(
	VulkanRenderer PRIVATE 
	Aureolab
)

target_compile_features(VulkanRenderer PRIVATE cxx_std_20)