add_executable(DependencyTestBed 
	DependencyTestBed.cpp
	ObjectSelection.cpp
	UniformBuffers.cpp
)

target_link_libraries(
	DependencyTestBed PRIVATE 
	Aureolab
)