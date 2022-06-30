project(a64dbg)
cmake_minimum_required( VERSION 3.17 )
set (CMAKE_CXX_STANDARD 17)

# TODO: change to https://github.com/build-cpp/cmkr

set(SOURCES
    src/a64dbg.cpp
    src/src/CmdParser.cpp
    src/src/Connector.cpp
    src/src/Utils/Utils.cpp
    src/src/Utils/Memory.cpp
    src/src/Plugins/Plugins.cpp
    src/src/Plugins/PUtils.cpp
)

add_subdirectory(plugins)

message("\n(a64dbg) enabled plugins:")
foreach (_variableName ${PLUGIN_SOURCES})
    message(STATUS "${_variableName}")
endforeach()
message("")

add_executable(${PROJECT_NAME} ${SOURCES} ${PLUGIN_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/include)
