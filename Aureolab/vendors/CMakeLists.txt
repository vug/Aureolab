include(FetchContent)
set(FETCHCONTENT_QUIET OFF) # turn on for getting debug information about a specific dependency

add_subdirectory(spdlog)
add_subdirectory(glfw)
add_subdirectory(glad)