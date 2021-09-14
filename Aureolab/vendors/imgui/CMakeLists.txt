# ImGui https://github.com/ocornut/imgui
# Immediate Mode GUI. "bloat-free graphical user interface library for C++"

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 8dfb52245bc92fef3e58b14f88135bd75a28faf8 # docking branch latest (> v1.84.2)
)

FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    add_subdirectory(${imgui_SOURCE_DIR})
endif()


# ImGui does not come with a CMakeLists.txt file. We have to prepare it as a library manually.
add_library(
    imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp # for ImGui::ShowDemoWindow()
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_compile_options(
    imgui PRIVATE
    -DIMGUI_IMPL_OPENGL_LOADER_GLAD # configure GLAD as the loader
)

target_link_libraries(
	imgui PRIVATE 
	glfw
)

target_include_directories(
    imgui
    PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    PRIVATE
    ${glad_BINARY_DIR}/include # so imgui can find glad
    ${glfw_SOURCE_DIR}/include # so imgui can find GLFW
)