add_executable(TestBed 
	TestBed.cpp
	Layer1.h Layer2.h Layer3.h
)

target_link_libraries(
	TestBed PRIVATE 
	Aureolab
)

target_include_directories(TestBed PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)


add_custom_target(copy_aureolab_assets_to_testbed
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Aureolab/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(TestBed copy_aureolab_assets_to_testbed)

add_custom_target(copy_test_bed_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(TestBed copy_test_bed_assets)

target_compile_features(TestBed PRIVATE cxx_std_17)