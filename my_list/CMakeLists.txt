cmake_minimum_required ( VERSION 3.28 )

project ( "my_list" )

set ( CMAKE_CXX_STANDARD 17 )
set ( CMAKE_CXX_STANDARD_REQUIRED true )

if ( NOT CMAKE_BUILD_TYPE )
  set ( CMAKE_BUILD_TYPE Debug )
endif ()

add_executable ( test1 src/test.cpp )