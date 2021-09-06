add_executable(TestBed 
	TestBed.cpp
)

target_link_libraries(
	TestBed PRIVATE 
	Aureolab
)

target_include_directories(TestBed PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)


add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)

add_dependencies(TestBed copy_assets)