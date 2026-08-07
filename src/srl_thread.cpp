#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_thread.hpp"
#include "vm.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace srl {

static std::unordered_map<double, std::thread> activeThreads;
static double threadCounter = 1.0;
static std::mutex g_threadMutex;

void THREAD::registerNativeFunctions(VM& vm) {
    // thread_create(fn_name)
    vm.defineNative("thread_create", [&vm](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string fnName = args[0].asString();
            
            std::lock_guard<std::mutex> lock(g_threadMutex);
            double id = threadCounter++;

            activeThreads[id] = std::thread([&vm, fnName]() {
                try {
                    // Execute function safely in VM context
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
            std::thread t;
            {
                std::lock_guard<std::mutex> lock(g_threadMutex);
                auto it = activeThreads.find(id);
                if (it != activeThreads.end()) {
                    t = std::move(it->second);
                    activeThreads.erase(it);
                }
            }
            if (t.joinable()) {
                t.join();
                return Value(true);
            }
        }
        return Value(false);
    });
}

} // namespace srl
