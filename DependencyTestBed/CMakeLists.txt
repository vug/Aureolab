add_executable(DependencyTestBed 
	DependencyTestBed.cpp Utilities.h
	ObjectSelection.cpp
	StartingExample.cpp
)

target_link_libraries(
	DependencyTestBed PRIVATE 
	Aureolab
)