cmake_minimum_required(VERSION 3.31)
project(quant LANGUAGES C CXX VERSION 1.0.0 )

find_package(Boost REQUIRED)
find_package(QuantLib REQUIRED)

message("Building with CMake version: ${CMAKE_VERSION}")
message("CMAKE_CURRENT_SOURCE_DIR='${CMAKE_CURRENT_SOURCE_DIR}'")
message("CMAKE_SOURCE_DIR='${CMAKE_SOURCE_DIR}'")

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC 
.
)

target_link_libraries(${PROJECT_NAME} boost::boost QuantLib::QuantLib)