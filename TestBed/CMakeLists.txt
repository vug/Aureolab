add_executable(TestBed 
	TestBed.cpp
)

target_link_libraries(
	TestBed PRIVATE 
	Aureolab
)

target_include_directories(TestBed PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)
