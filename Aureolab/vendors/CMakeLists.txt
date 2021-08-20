include(FetchContent)
set(FETCHCONTENT_QUIET off) # turn on for getting debug information about a specific dependency


# spdlog
FetchContent_Declare(
    spdlog 
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG eb3220622e73a4889eee355ffa37972b3cac3df5 # v1.9.2 (Using hash skips connection to git server if downloaded)
)
#FetchContent_MakeAvailable(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)

    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL skips binary generation
endif()


# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 814b7929c5add4b0541ccad26fb81f28b71dc4d8 # release 3.3.4
)
#FetchContent_MakeAvailable(glfw)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()
