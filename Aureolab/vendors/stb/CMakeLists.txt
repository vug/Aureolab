# stb https://github.com/nothings/stb
# "single-file public domain (or MIT licensed) libraries for C/C++"

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a # latest (no releases)
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)

    #add_subdirectory(${stb_SOURCE_DIR}) # because there is no CMakeLists.txt file in stb repo (?)
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR}/)