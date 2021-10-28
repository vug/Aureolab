add_executable(VulkanStudy 
	main.cpp
)

target_link_libraries(
	VulkanStudy PRIVATE 
	Aureolab
)

target_compile_features(VulkanStudy PRIVATE cxx_std_20)