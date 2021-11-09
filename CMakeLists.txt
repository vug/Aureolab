cmake_minimum_required(VERSION 3.12)

project(Aureolab 
        VERSION 0.1
        DESCRIPTION "An engine & editor for studying computer graphics")

add_subdirectory(Aureolab/vendors)
add_subdirectory(Aureolab/src)
add_subdirectory(TestBed)
add_subdirectory(DependencyTestBed)
add_subdirectory(ShaderBoy)
add_subdirectory(Editor)
add_subdirectory(VulkanStudy)
add_subdirectory(VulkanRenderer)
