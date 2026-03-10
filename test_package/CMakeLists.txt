cmake_minimum_required(VERSION 3.15)

project(StructifyTester
        DESCRIPTION "Tester package for structify"
        LANGUAGES C CXX)

find_package(structify REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} structify::structify)
