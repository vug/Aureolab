include(FetchContent)
set(FETCHCONTENT_QUIET off) # turn on for getting debug information about a specific dependency

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

#TODO: add GLFW dependency
#FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
#FetchContent_MakeAvailable(glfw)
