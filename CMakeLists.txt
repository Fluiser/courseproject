set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE "F:/github/vcpkg/scripts/buildsystems/vcpkg.cmake")
cmake_minimum_required(VERSION 3.11)
project(g)

add_executable(g main.cpp "lib/imgui-SFML.cpp")

#compile config
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

#SFML
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
find_package(imgui REQUIRED)

target_link_libraries(g PUBLIC SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System imgui::imgui opengl32)
