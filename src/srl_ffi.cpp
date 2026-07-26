#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_ffi.hpp"
#include "vm.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#endif

namespace srl {

static std::unordered_map<double, void*> loadedLibraries;
static double handleCounter = 1.0;

void FFI::registerNativeFunctions(VM& vm) {
    // ffi_load(dll_path)
    vm.defineNative("ffi_load", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (argCount > 0 && args[0].isString()) {
            HMODULE handle = LoadLibraryA(args[0].asString().c_str());
            if (handle) {
                double id = handleCounter++;
                loadedLibraries[id] = static_cast<void*>(handle);
                return Value(id);
            }
        }
#endif
        return Value(0.0);
    });

    // ffi_free(handle)
    vm.defineNative("ffi_free", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            auto it = loadedLibraries.find(id);
            if (it != loadedLibraries.end()) {
                FreeLibrary(static_cast<HMODULE>(it->second));
                loadedLibraries.erase(it);
                return Value(true);
            }
        }
#endif
        return Value(false);
    });

    // ffi_call(handle, symbol_name, return_type, args_array)
    vm.defineNative("ffi_call", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (argCount >= 3 && args[0].isNumber() && args[1].isString() && args[2].isString()) {
            double id = args[0].asNumber();
            std::string funcName = args[1].asString();
            std::string retType = args[2].asString();

            auto it = loadedLibraries.find(id);
            if (it == loadedLibraries.end() || !it->second) return Value();

            FARPROC proc = GetProcAddress(static_cast<HMODULE>(it->second), funcName.c_str());
            if (!proc) {
                std::cerr << "[FFI Error] Symbol '" << funcName << "' not found in DLL." << std::endl;
                return Value();
            }

            // Simple void signature call
            typedef int (*GenericFn)();
            GenericFn fn = (GenericFn)proc;
            int res = fn();

            if (retType == "int" || retType == "number") {
                return Value(static_cast<double>(res));
            } else if (retType == "bool") {
                return Value(res != 0);
            }
            return Value();
        }
#endif
        return Value();
    });
}

} // namespace srl
