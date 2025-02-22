cmake_minimum_required(VERSION 3.31)
project(sim-poco LANGUAGES C CXX VERSION 1.0.0 )

find_package(Poco REQUIRED)
message("Building with CMake version: ${CMAKE_VERSION}")
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} Poco::Poco)