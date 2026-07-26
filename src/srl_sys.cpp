#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_sys.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <array>
#include <filesystem>


#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace srl {

void SYS::registerNativeFunctions(VM& vm) {
    // sys_exec(cmd) -> returns command stdout string!
    vm.defineNative("sys_exec", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string cmd = args[0].asString();
            std::string result;
#ifdef _WIN32
            FILE* pipe = _popen(cmd.c_str(), "r");
            if (pipe) {
                std::array<char, 256> buffer;
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                    result += buffer.data();
                }
                _pclose(pipe);
            }
#endif
            return Value(result);
        }
        return Value("");
    });

    // sys_env_get(var)
    vm.defineNative("sys_env_get", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            const char* val = std::getenv(args[0].asString().c_str());
            if (val) return Value(std::string(val));
        }
        return Value("");
    });

    // sys_env_set(var, val)
    vm.defineNative("sys_env_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
#ifdef _WIN32
            _putenv_s(args[0].asString().c_str(), args[1].asString().c_str());
            return Value(true);
#endif
        }
        return Value(false);
    });

    // sys_cpu_count()
    vm.defineNative("sys_cpu_count", [](int argCount, const Value* args) -> Value {
        unsigned int cores = std::thread::hardware_concurrency();
        return Value(static_cast<double>(cores));
    });

    // sys_memory_usage()
    vm.defineNative("sys_memory_usage", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return Value(static_cast<double>(pmc.WorkingSetSize)); // in bytes
        }
#endif
        return Value(0.0);
    });

    // sys_pid()
    vm.defineNative("sys_pid", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        return Value(static_cast<double>(GetCurrentProcessId()));
#else
        return Value(0.0);
#endif
    });

    // sys_os()
    vm.defineNative("sys_os", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        return Value("Windows");
#elif __APPLE__
        return Value("macOS");
#else
        return Value("Linux");
#endif
    });

    // sys_pwd()
    vm.defineNative("sys_pwd", [](int argCount, const Value* args) -> Value {
        std::error_code ec;
        return Value(std::filesystem::current_path(ec).string());
    });
}

} // namespace srl

