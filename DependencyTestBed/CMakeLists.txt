add_executable(DependencyTestBed 
	DependencyTestBed.cpp Utilities.h
	"ObjectSelection.h"
	"StartingExample.h"
	"UniformBuffer.h"
)

target_link_libraries(
	DependencyTestBed PRIVATE 
	Aureolab
)

target_compile_features(DependencyTestBed PRIVATE cxx_std_20)