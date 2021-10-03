add_executable(Editor 
	Editor.cpp
	EditorLayer.h EditorLayer.cpp
    Panels/SceneHierarchyPanel.h Panels/SceneHierarchyPanel.cpp
    Panels/InspectorPanel.h Panels/InspectorPanel.cpp
)

target_link_libraries(
	Editor PRIVATE 
	Aureolab
)

target_include_directories(Editor PRIVATE
	${CMAKE_SOURCE_DIR}/Aureolab/src)

add_custom_target(copy_aureolab_assets_to_editor
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Aureolab/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(Editor copy_aureolab_assets_to_editor)

target_compile_features(Editor PRIVATE cxx_std_20)