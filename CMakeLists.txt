cmake_minimum_required(VERSION 3.26)
project(sandbox)

set(CMAKE_CXX_STANDARD 23)

find_package(flecs CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(sandbox main.cpp)

target_link_libraries(sandbox
        PRIVATE
        flecs::flecs
        flecs::flecs_static
        glm::glm
)