# Cereal - A C++11 library for serialization: https://github.com/g-truc/glm
# "cereal takes arbitrary data types and reversibly turns them into different representations, such as compact binary encodings, XML, or JSON."

FetchContent_Declare(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal.git
    GIT_TAG af0700efb25e7dc7af637b9e6f970dbb94813bff # latest (> v1.3.0)
)

FetchContent_GetProperties(cereal)
if(NOT cereal_POPULATED)
    FetchContent_Populate(cereal)

    set(SKIP_PERFORMANCE_COMPARISON ON) # otherwise it requires Boost

    add_subdirectory(${cereal_SOURCE_DIR} ${cereal_BINARY_DIR})
endif()