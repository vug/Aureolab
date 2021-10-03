# ImGuizmo https://github.com/CedricGuillemet/ImGuizmo
# "ImGizmo is a small (.h and .cpp) library built ontop of Dear ImGui that allow you to manipulate (Rotate & translate at the moment) 4x4 float matrices."

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG 7c16cac47891ee0cb14641b1699cb5f38ce4a7d3 # docking branch latest (> v1.83)
)

FetchContent_MakeAvailable(imguizmo)
FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)

    add_subdirectory(${imguizmo_SOURCE_DIR})
endif()

# ImGuizmo does not come with a CMakeLists.txt file. We have to prepare it as a library manually.
add_library(
    imguizmo
    ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)

target_link_libraries(
	imguizmo PRIVATE 
	imgui
)

target_include_directories(
    imguizmo
    PUBLIC
    ${imguizmo_SOURCE_DIR}
)
