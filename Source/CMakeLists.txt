cmake_minimum_required(VERSION 3.6)
project(Source CXX)

include(../DevEnvironment/CMake/third_party.cmake)
include(../DevEnvironment/CMake/game_project.cmake)
include(GenerateExportHeader)

add_subdirectory(Engine)
add_subdirectory(HelloWorld)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(MSVC)
    set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT HelloWorld)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HelloWorld)
    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "3 CMake")
endif()
