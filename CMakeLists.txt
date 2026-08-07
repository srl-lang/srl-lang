cmake_minimum_required(VERSION 3.15)
project(srl_lang VERSION 0.3.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(srl ${SOURCES})

if(WIN32)
    target_link_libraries(srl winmm comdlg32 ws2_32 gdi32 user32 psapi comctl32 shell32 ole32 shlwapi Threads::Threads)
    if(MSVC)
        target_link_options(srl PRIVATE "/MANIFESTUAC:level='asInvoker'")
    endif()
else()
    target_link_libraries(srl ${CMAKE_DL_LIBS} Threads::Threads)
endif()

# ---------------------------------------------------------------------------
# Standalone Native Modern GUI Installer Target (srl-installer.exe)
# ---------------------------------------------------------------------------
add_executable(srl-installer
    src/installer.cpp
    src/installer_gui.cpp
    src/cli.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp
    src/value.cpp
    src/dsp.cpp
    src/srl_audio.cpp
    src/srl_gui.cpp
    src/srl_ffi.cpp
    src/srl_gfx.cpp
    src/srl_net.cpp
    src/srl_sys.cpp
    src/srl_thread.cpp
    src/srl_json.cpp
    src/srl_db.cpp
    src/srl_crypto.cpp
    src/watcher.cpp
    src/jit.cpp
)

target_compile_definitions(srl-installer PRIVATE SRL_STANDALONE_INSTALLER)
if(WIN32)
    target_link_libraries(srl-installer winmm comdlg32 ws2_32 gdi32 user32 psapi comctl32 shell32 ole32 shlwapi Threads::Threads)
else()
    target_link_libraries(srl-installer ${CMAKE_DL_LIBS} Threads::Threads)
endif()



# ---------------------------------------------------------------------------
# Native plugin targets (optional - each builds as a standalone DLL/SO)
# Add new plugins here with add_subdirectory(plugins/<name>)
# Plugins do NOT depend on srl.exe and can be built/distributed independently.
# ---------------------------------------------------------------------------
option(SRL_BUILD_PLUGINS "Build bundled native plugin DLLs" ON)

if(SRL_BUILD_PLUGINS)
    add_subdirectory(plugins/example_plugin)
    add_subdirectory(plugins/srl_audio_plugin)
endif()

# ---------------------------------------------------------------------------
# Installation Targets
# ---------------------------------------------------------------------------
install(TARGETS srl DESTINATION bin)
install(DIRECTORY std/     DESTINATION std)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY plugins/  DESTINATION plugins)
