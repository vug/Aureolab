# Download Vulkan SDK from https://vulkan.lunarg.com/

# This is a configuration hard-coded for my Windows computer only, and it is not portable. 
# Probably better to ask location of vulkan library and headers when invoking CMake command instead of using absolute paths here.
add_library(vulkan STATIC IMPORTED GLOBAL)
set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION "C:\\VulkanSDK\\1.2.189.2\\Lib\\vulkan-1.lib")
target_include_directories(vulkan INTERFACE "C:\\VulkanSDK\\1.2.189.2\\Include")