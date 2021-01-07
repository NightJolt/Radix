cmake_minimum_required(VERSION 3.17)
project(Radix)

set(CMAKE_CXX_STANDARD 20)
#set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

include_directories(header-files)
include_directories(source-files)

file (GLOB_RECURSE SOURCE_FILES header-files/*.h source-files/*.cpp)

add_executable(Radix ${SOURCE_FILES})