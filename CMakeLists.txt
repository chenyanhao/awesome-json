cmake_minimum_required(VERSION 3.8)
project(awesome_json)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES test.cpp awesomejson.h awesomejson.cpp)
add_executable(awesome_json ${SOURCE_FILES})