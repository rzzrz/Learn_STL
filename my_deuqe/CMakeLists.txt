cmake_minimum_required ( VERSION 3.28 )

project ( "my_deque" )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set ( CMAKE_CXX_STANDARD 17 )
set ( CMAKE_CXX_STANDARD_REQUIRED true )

find_package(glog REQUIRED)

if ( NOT CMAKE_BUILD_TYPE )
  set ( CMAKE_BUILD_TYPE Debug )
endif ()

add_executable ( test1 src/test.cpp )

#find_package(glog REQUIRED)
target_link_libraries(test1 PRIVATE glog::glog)