project(a64dbg_tests)
cmake_minimum_required( VERSION 3.10 )
set (CMAKE_CXX_STANDARD 17)

set(SOURCES
    readfile.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_executable(a64dbg_attach loop_test.cpp)

