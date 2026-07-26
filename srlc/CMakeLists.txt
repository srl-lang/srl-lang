cmake_minimum_required(VERSION 3.15)
project(srlc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include ../include)

add_executable(srlc
    src/main.cpp
    src/llvm_codegen.cpp
    ../src/lexer.cpp
    ../src/parser.cpp
    ../src/compiler.cpp
    ../src/value.cpp
    ../src/dsp.cpp
    ../src/vm.cpp
    ../src/watcher.cpp
)
