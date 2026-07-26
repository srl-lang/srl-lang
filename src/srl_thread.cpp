#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_thread.hpp"
#include "vm.hpp"
#include <iostream>
#include <thread>
#include <unordered_map>

namespace srl {

static std::unordered_map<double, std::thread> activeThreads;
static double threadCounter = 1.0;

void THREAD::registerNativeFunctions(VM& vm) {
    // thread_create(fn_name)
    vm.defineNative("thread_create", [&vm](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string fnName = args[0].asString();
            double id = threadCounter++;

            activeThreads[id] = std::thread([&vm, fnName]() {
                try {
                    vm.runFunction(fnName);
                } catch (...) {}
            });

            return Value(id);
        }
        return Value(0.0);
    });

    // thread_join(thread_id)
    vm.defineNative("thread_join", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            auto it = activeThreads.find(id);
            if (it != activeThreads.end()) {
                if (it->second.joinable()) {
                    it->second.join();
                }
                activeThreads.erase(it);
                return Value(true);
            }
        }
        return Value(false);
    });
}

} // namespace srl
