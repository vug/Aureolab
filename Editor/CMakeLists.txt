add_executable(Editor 
	Editor.cpp
	EditorLayer.h
)

target_link_libraries(
	Editor PRIVATE 
	Aureolab
)

target_include_directories(Editor PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)

# add_custom_target(copy_assets
#	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
#
#
#add_dependencies(TestBed copy_assets)

target_compile_features(Editor PRIVATE cxx_std_17)