cmake_minimum_required(VERSION 3.15)
project(srl_lang VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(srl ${SOURCES})

if(WIN32)
    target_link_libraries(srl winmm comdlg32 ws2_32 gdi32 user32 psapi Threads::Threads)
else()
    target_link_libraries(srl ${CMAKE_DL_LIBS} Threads::Threads)
endif()

# Installation Targets
install(TARGETS srl DESTINATION bin)
install(DIRECTORY std/ DESTINATION std)
install(DIRECTORY include/ DESTINATION include)


