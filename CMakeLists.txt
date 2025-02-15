cmake_minimum_required(VERSION 3.10)
project(SmartPtrs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADER_FILES "include/*.h")

add_library(smart_ptrs INTERFACE)
target_include_directories(smart_ptrs INTERFACE ${CMAKE_SOURCE_DIR}/include)

