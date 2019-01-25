cmake_minimum_required(VERSION 3.10)
project(functionImpl)

set(CMAKE_CXX_STANDARD 14)

add_executable(functionImpl main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -D__GLIBCXX_DEBUG -Wall -Wextra")