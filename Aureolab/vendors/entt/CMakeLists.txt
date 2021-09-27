# Entt - Entity Component System (ECS): https://github.com/skypjack/entt
# "EnTT is a header-only, tiny and easy to use library for game programming and much more written in modern C++."

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG 4b3bd265a67c050884d4a98ec141ed27ce642c67 # latest (> v3.8.1)
)

FetchContent_GetProperties(entt)
if(NOT entt_POPULATED)
    FetchContent_Populate(entt)

    add_subdirectory(${entt_SOURCE_DIR} ${entt_BINARY_DIR})
endif()