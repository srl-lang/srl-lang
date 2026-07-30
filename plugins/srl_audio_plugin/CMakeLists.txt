cmake_minimum_required(VERSION 3.15)
project(srl_audio_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

add_library(srl_audio SHARED srl_audio_plugin.cpp)
target_include_directories(srl_audio PRIVATE "${SRL_INCLUDE_DIR}")
set_target_properties(srl_audio PROPERTIES PREFIX "")

if(WIN32)
    target_link_libraries(srl_audio winmm)
    target_compile_definitions(srl_audio PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

install(TARGETS srl_audio DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
