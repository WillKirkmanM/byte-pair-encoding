cmake_minimum_required(VERSION 3.10)

project(BytePairEncoding CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bpe_tool main.cpp)

target_link_libraries(bpe_tool PRIVATE stdc++fs)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bpe_tool PRIVATE -Wall -Wextra -pedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(bpe_tool PRIVATE /W4)
endif()

