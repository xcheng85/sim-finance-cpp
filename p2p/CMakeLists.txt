cmake_minimum_required(VERSION 3.31)
project(p2p LANGUAGES C CXX VERSION 1.0.0 )

find_package(Boost REQUIRED)

message("Building with CMake version: ${CMAKE_VERSION}")
add_executable(${PROJECT_NAME} src/signaling.cpp src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} boost::boost)