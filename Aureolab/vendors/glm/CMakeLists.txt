# OpenGL Mathematics (GLM): https://github.com/g-truc/glm
# "a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications."

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 06ed280db4e274fa5e1f36d5ea4f7dfd654ff9b0 # latest (> v0.9.9.8)
)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)

    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()
