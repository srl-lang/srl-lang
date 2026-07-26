// Updated SRL FFI Engine
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_ffi.hpp"
#include "vm.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace srl {

static std::unordered_map<double, void*> loadedLibraries;
static double handleCounter = 1.0;

typedef bool (*ModuleInitFn)(void* vm_ptr);

void FFI::registerNativeFunctions(VM& vm) {
    // ---------------------------------------------------------
    // ffi_load(dll_path) -> handle (number)
    // ---------------------------------------------------------
    vm.defineNative("ffi_load", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string path = args[0].asString();
            void* handle = nullptr;
#ifdef _WIN32
            handle = static_cast<void*>(LoadLibraryA(path.c_str()));
#else
            handle = dlopen(path.c_str(), RTLD_LAZY);
#endif
            if (handle) {
                double id = handleCounter++;
                loadedLibraries[id] = handle;
                return Value(id);
            } else {
                std::cerr << "[FFI Error] Failed to load shared library: " << path << std::endl;
            }
        }
        return Value(0.0);
    });

    // ---------------------------------------------------------
    // ffi_free(handle) -> bool
    // ---------------------------------------------------------
    vm.defineNative("ffi_free", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            auto it = loadedLibraries.find(id);
            if (it != loadedLibraries.end()) {
                void* handle = it->second;
                if (handle) {
#ifdef _WIN32
                    FreeLibrary(static_cast<HMODULE>(handle));
#else
                    dlclose(handle);
#endif
                }
                loadedLibraries.erase(it);
                return Value(true);
            }
        }
        return Value(false);
    });

    // ---------------------------------------------------------
    // ffi_call(handle, symbol_name, return_type, [param_types], [args])
    // ---------------------------------------------------------
    vm.defineNative("ffi_call", [](int argCount, const Value* args) -> Value {
        if (argCount < 3 || !args[0].isNumber() || !args[1].isString() || !args[2].isString()) {
            std::cerr << "[FFI Error] Usage: ffi_call(handle, symbol_name, return_type, [param_types], [args])" << std::endl;
            return Value();
        }

        double id = args[0].asNumber();
        std::string funcName = args[1].asString();
        std::string retType = args[2].asString();

        auto it = loadedLibraries.find(id);
        if (it == loadedLibraries.end() || !it->second) {
            std::cerr << "[FFI Error] Invalid or null library handle ID: " << id << std::endl;
            return Value();
        }

        void* proc = nullptr;
#ifdef _WIN32
        proc = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(it->second), funcName.c_str()));
#else
        proc = dlsym(it->second, funcName.c_str());
#endif

        if (!proc) {
            std::cerr << "[FFI Error] Symbol '" << funcName << "' not found in library." << std::endl;
            return Value();
        }

        // Collect argument values as intptr_t and double arrays
        std::vector<intptr_t> rawArgs;
        std::vector<double> doubleArgs;
        std::vector<std::string> tempStrings; // keep string buffers alive

        ArrayPtr argArray = nullptr;
        if (argCount >= 5 && args[4].isArray()) {
            argArray = args[4].asArray();
        } else if (argCount >= 4 && args[3].isArray()) {
            argArray = args[3].asArray();
        }

        if (argArray) {
            for (size_t i = 0; i < argArray->size(); ++i) {
                const Value& v = (*argArray)[i];
                if (v.isNumber()) {
                    rawArgs.push_back(static_cast<intptr_t>(v.asNumber()));
                    doubleArgs.push_back(v.asNumber());
                } else if (v.isString()) {
                    tempStrings.push_back(v.asString());
                    rawArgs.push_back(reinterpret_cast<intptr_t>(tempStrings.back().c_str()));
                    doubleArgs.push_back(0.0);
                } else if (v.isBool()) {
                    rawArgs.push_back(v.asBool() ? 1 : 0);
                    doubleArgs.push_back(v.asBool() ? 1.0 : 0.0);
                } else {
                    rawArgs.push_back(0);
                    doubleArgs.push_back(0.0);
                }
            }
        } else {
            // Positional arguments starting at index 3
            for (int i = 3; i < argCount; ++i) {
                const Value& v = args[i];
                if (v.isNumber()) {
                    rawArgs.push_back(static_cast<intptr_t>(v.asNumber()));
                    doubleArgs.push_back(v.asNumber());
                } else if (v.isString()) {
                    tempStrings.push_back(v.asString());
                    rawArgs.push_back(reinterpret_cast<intptr_t>(tempStrings.back().c_str()));
                    doubleArgs.push_back(0.0);
                } else if (v.isBool()) {
                    rawArgs.push_back(v.asBool() ? 1 : 0);
                    doubleArgs.push_back(v.asBool() ? 1.0 : 0.0);
                } else {
                    rawArgs.push_back(0);
                    doubleArgs.push_back(0.0);
                }
            }
        }

        // Dispatch based on parameter count & return type
        size_t nArgs = rawArgs.size();
        intptr_t res = 0;

        if (retType == "double" || retType == "float") {
            typedef double (*Fn0D)();
            typedef double (*Fn1DD)(double);
            typedef double (*Fn2DD)(double, double);

            double dRes = 0.0;
            if (nArgs == 0) {
                dRes = ((Fn0D)proc)();
            } else if (nArgs == 1) {
                dRes = ((Fn1DD)proc)(doubleArgs[0]);
            } else if (nArgs == 2) {
                dRes = ((Fn2DD)proc)(doubleArgs[0], doubleArgs[1]);
            } else {
                std::cerr << "[FFI Error] Unsupported argument count (" << nArgs << ") for double return." << std::endl;
                return Value();
            }
            return Value(dRes);
        }

        typedef intptr_t (*Fn0)();
        typedef intptr_t (*Fn1)(intptr_t);
        typedef intptr_t (*Fn2)(intptr_t, intptr_t);
        typedef intptr_t (*Fn3)(intptr_t, intptr_t, intptr_t);
        typedef intptr_t (*Fn4)(intptr_t, intptr_t, intptr_t, intptr_t);
        typedef intptr_t (*Fn5)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
        typedef intptr_t (*Fn6)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);

        switch (nArgs) {
            case 0: res = ((Fn0)proc)(); break;
            case 1: res = ((Fn1)proc)(rawArgs[0]); break;
            case 2: res = ((Fn2)proc)(rawArgs[0], rawArgs[1]); break;
            case 3: res = ((Fn3)proc)(rawArgs[0], rawArgs[1], rawArgs[2]); break;
            case 4: res = ((Fn4)proc)(rawArgs[0], rawArgs[1], rawArgs[2], rawArgs[3]); break;
            case 5: res = ((Fn5)proc)(rawArgs[0], rawArgs[1], rawArgs[2], rawArgs[3], rawArgs[4]); break;
            case 6: res = ((Fn6)proc)(rawArgs[0], rawArgs[1], rawArgs[2], rawArgs[3], rawArgs[4], rawArgs[5]); break;
            default:
                std::cerr << "[FFI Error] Too many arguments (" << nArgs << "), max supported is 6." << std::endl;
                return Value();
        }

        if (retType == "int" || retType == "number" || retType == "pointer") {
            return Value(static_cast<double>(res));
        } else if (retType == "bool") {
            return Value(res != 0);
        } else if (retType == "string") {
            if (res != 0) {
                return Value(std::string(reinterpret_cast<const char*>(res)));
            }
            return Value("");
        } else if (retType == "void") {
            return Value();
        }

        return Value(static_cast<double>(res));
    });

    // ---------------------------------------------------------
    // ffi_malloc(size_bytes) -> pointer_address (number)
    // ---------------------------------------------------------
    vm.defineNative("ffi_malloc", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            size_t sz = static_cast<size_t>(args[0].asNumber());
            void* ptr = std::malloc(sz);
            if (ptr) {
                std::memset(ptr, 0, sz);
                return Value(static_cast<double>(reinterpret_cast<uintptr_t>(ptr)));
            }
        }
        return Value(0.0);
    });

    // ---------------------------------------------------------
    // ffi_free_mem(address) -> bool
    // ---------------------------------------------------------
    vm.defineNative("ffi_free_mem", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            uintptr_t addr = static_cast<uintptr_t>(args[0].asNumber());
            if (addr != 0) {
                std::free(reinterpret_cast<void*>(addr));
                return Value(true);
            }
        }
        return Value(false);
    });

    // ---------------------------------------------------------
    // ffi_read_string(address) -> string
    // ---------------------------------------------------------
    vm.defineNative("ffi_read_string", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            uintptr_t addr = static_cast<uintptr_t>(args[0].asNumber());
            if (addr != 0) {
                const char* str = reinterpret_cast<const char*>(addr);
                return Value(std::string(str));
            }
        }
        return Value("");
    });

    // ---------------------------------------------------------
    // ffi_write_string(address, string_val) -> bool
    // ---------------------------------------------------------
    vm.defineNative("ffi_write_string", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isString()) {
            uintptr_t addr = static_cast<uintptr_t>(args[0].asNumber());
            std::string src = args[1].asString();
            if (addr != 0) {
                char* dst = reinterpret_cast<char*>(addr);
                std::memcpy(dst, src.c_str(), src.size() + 1);
                return Value(true);
            }
        }
        return Value(false);
    });

    // ---------------------------------------------------------
    // ffi_read_number(address, type_name) -> number
    // ---------------------------------------------------------
    vm.defineNative("ffi_read_number", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isNumber()) {
            uintptr_t addr = static_cast<uintptr_t>(args[0].asNumber());
            std::string typeName = (argCount >= 2 && args[1].isString()) ? args[1].asString() : "int";
            if (addr != 0) {
                if (typeName == "double") {
                    return Value(*reinterpret_cast<double*>(addr));
                } else if (typeName == "float") {
                    return Value(static_cast<double>(*reinterpret_cast<float*>(addr)));
                } else {
                    return Value(static_cast<double>(*reinterpret_cast<int*>(addr)));
                }
            }
        }
        return Value(0.0);
    });

    // ---------------------------------------------------------
    // ffi_write_number(address, value, type_name) -> bool
    // ---------------------------------------------------------
    vm.defineNative("ffi_write_number", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            uintptr_t addr = static_cast<uintptr_t>(args[0].asNumber());
            double val = args[1].asNumber();
            std::string typeName = (argCount >= 3 && args[2].isString()) ? args[2].asString() : "int";
            if (addr != 0) {
                if (typeName == "double") {
                    *reinterpret_cast<double*>(addr) = val;
                } else if (typeName == "float") {
                    *reinterpret_cast<float*>(addr) = static_cast<float>(val);
                } else {
                    *reinterpret_cast<int*>(addr) = static_cast<int>(val);
                }
                return Value(true);
            }
        }
        return Value(false);
    });

    // ---------------------------------------------------------
    // import_native(dll_path) -> bool (Native C++ Extension Loader)
    // ---------------------------------------------------------
    vm.defineNative("import_native", [&vm](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string path = args[0].asString();
            void* handle = nullptr;
#ifdef _WIN32
            handle = static_cast<void*>(LoadLibraryA(path.c_str()));
#else
            handle = dlopen(path.c_str(), RTLD_LAZY);
#endif
            if (!handle) {
                std::cerr << "[Native Plugin Error] Failed to load dynamic plugin module: " << path << std::endl;
                return Value(false);
            }

            ModuleInitFn initFn = nullptr;
#ifdef _WIN32
            initFn = reinterpret_cast<ModuleInitFn>(GetProcAddress(static_cast<HMODULE>(handle), "srl_module_init"));
            if (!initFn) {
                initFn = reinterpret_cast<ModuleInitFn>(GetProcAddress(static_cast<HMODULE>(handle), "srl_plugin_init"));
            }
#else
            initFn = reinterpret_cast<ModuleInitFn>(dlsym(handle, "srl_module_init"));
            if (!initFn) {
                initFn = reinterpret_cast<ModuleInitFn>(dlsym(handle, "srl_plugin_init"));
            }
#endif

            if (!initFn) {
                std::cerr << "[Native Plugin Error] Symbol 'srl_module_init' not found in: " << path << std::endl;
                return Value(false);
            }

            bool ok = initFn(static_cast<void*>(&vm));
            if (ok) {
                double id = handleCounter++;
                loadedLibraries[id] = handle;
            }
            return Value(ok);
        }
        return Value(false);
    });
}

} // namespace srl
