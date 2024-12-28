cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

project(AgroMaster)

find_package(Wt REQUIRED)

file(GLOB_RECURSE INC "source/*.hpp")
file(GLOB_RECURSE SRC "source/*.cpp")

add_executable(agromaster ${INC} ${SRC})

target_link_libraries(agromaster Wt::Wt)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INC} ${SRC})