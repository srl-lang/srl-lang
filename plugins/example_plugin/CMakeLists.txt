cmake_minimum_required(VERSION 3.15)
project(example_plugin LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate SRL include dir (two levels up from this file)
set(SRL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

add_library(example_plugin SHARED example_plugin.cpp)
target_include_directories(example_plugin PRIVATE "${SRL_INCLUDE_DIR}")

# Strip the default 'lib' prefix on Linux so the filename matches what SRL expects
set_target_properties(example_plugin PROPERTIES PREFIX "")

# Windows: suppress default console subsystem
if(WIN32)
    target_compile_definitions(example_plugin PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# Install the DLL/SO next to the source for easy import_native testing
install(TARGETS example_plugin DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
