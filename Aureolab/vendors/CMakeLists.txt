include(FetchContent)
set(FETCHCONTENT_QUIET off)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git)
#FetchContent_MakeAvailable(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

#TODO: add GLFW dependency
#FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
#FetchContent_MakeAvailable(glfw)
