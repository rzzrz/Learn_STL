cmake_minimum_required(VERSION 3.12)
project(MemoryPool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main ./src/test.cpp ./src/memoryPool.cpp)