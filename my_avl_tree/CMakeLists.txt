cmake_minimun_requied(VERSION 2.28)

project(avl_tree)

set (CXX_STANDARD_REQUIRED true)
set (CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE )
  set (CMAKE_BUILD_TYPE Debug)
endif()

add_executable(main ./src/main)