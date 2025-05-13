cmake_minimum_required ( VERSION 3.28 )

project ( "my_vector" )

set ( CMAKE_CXX_STANDARD 11 )
set ( CMAKE_CXX_STANDARD_REQUIRED true )

if ( NOT CMAKE_BUILD_TYPE )
  set ( CMAKE_BUILD_TYPE Debug )
endif ()

add_executable ( main src/main.cpp )
