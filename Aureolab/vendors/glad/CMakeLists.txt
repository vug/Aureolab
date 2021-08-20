# glad https://gen.glad.sh/
# "Multi-Language Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator."

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG 71b2aa61f1f0ae94be5f2a7674275966aa15f8ad # latest (> v0.1.34)
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)

    # https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D4.6
    set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
    set(GLAD_API "gl=4.6" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
    set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the binding for")

    message("glad folders: ${glad_SOURCE_DIR} ${glad_BINARY_DIR}")
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR})
endif()