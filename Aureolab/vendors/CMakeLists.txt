include(FetchContent)
set(FETCHCONTENT_QUIET OFF) # turn on for getting debug information about a specific dependency

set(Vulkan_LIBRARY "C:\\VulkanSDK\\1.2.189.2\\Lib\\vulkan-1.lib")
set(Vulkan_INCLUDE_DIR "C:\\VulkanSDK\\1.2.189.2\\Include")

add_subdirectory(spdlog)
add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(glm)
add_subdirectory(imgui)
add_subdirectory(tinyobjloader)
add_subdirectory(entt)
add_subdirectory(cereal)
add_subdirectory(imguizmo)
add_subdirectory(vulkan)
add_subdirectory(stb)
add_subdirectory(vkmemalloc)