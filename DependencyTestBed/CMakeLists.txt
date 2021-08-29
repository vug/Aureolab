add_executable(DependencyTestBed 
	DependencyTestBed.cpp
)

target_link_libraries(
	DependencyTestBed PRIVATE 
	Aureolab
)