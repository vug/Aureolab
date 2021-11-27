# tinyobjloader
# "Tiny but powerful single file wavefront obj loader written in C++03."

FetchContent_Declare(
    tinyobjloader 
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG 3e401b55a4a9ebfc6344a8a2a698245e7d3fc09e # v2.0.0rc9 (Using hash skips connection to git server if downloaded)
)
#FetchContent_MakeAvailable(tinyobjloader)
if(NOT tinyobjloader_POPULATED)
    FetchContent_Populate(tinyobjloader)

    add_subdirectory(${tinyobjloader_SOURCE_DIR} ${tinyobjloader_BINARY_DIR})
endif()