cmake_minimum_required(VERSION 3.28)

project( "memoryPool" )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(main src/test.cpp)
add_executable(test_shared_ptr_with_pool src/shared_ptr_test.cpp)
