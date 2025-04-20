cmake_minimum_required(VERSION 2.28)

project(shared_resource)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 11)

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BINARY_TYPE Debug)
endif()

add_executable(test_sharedResource src/shared_resource.cpp)