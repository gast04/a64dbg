project(a64dbg)
cmake_minimum_required( VERSION 3.10 )
set (CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/a64dbg.cpp
    src/src/CmdParser.cpp
    src/src/Connector.cpp
    src/src/Utils.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/include)
