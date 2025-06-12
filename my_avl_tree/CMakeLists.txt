cmake_minimum_required ( VERSION 3.28 )

project( "avl_tree" )

set (CXX_STANDARD_REQUIRED true)
set (CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE )
  set (CMAKE_BUILD_TYPE Debug)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable ( main 
./src/main.cpp 
./src/memoryPool.cpp 
)
