# Vulkan Memory Allocator
# "Easy to integrate Vulkan memory allocation library.


FetchContent_Declare(
    vkmemalloc 
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG 00d064e21e596d1f2919266ae6d92c60bbe5855c # latest > v2.3.0+vs2017 (Using hash skips connection to git server if downloaded)
)
FetchContent_MakeAvailable(vkmemalloc)
if(NOT vkmemalloc_POPULATED)
    FetchContent_Populate(vkmemalloc)

    add_subdirectory(${vkmemalloc_SOURCE_DIR} ${vkmemalloc_BINARY_DIR})
endif()

# sometimes needed for header only libraries such as stb
# another option: https://github.com/usnistgov/hevx/blob/master/third_party/VulkanMemoryAllocator.cmake
add_library(vkmemalloc INTERFACE)
target_include_directories(vkmemalloc INTERFACE ${vkmemalloc_SOURCE_DIR}/include)