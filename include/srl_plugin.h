#ifndef SRL_PLUGIN_H
#define SRL_PLUGIN_H

#ifdef __cplusplus
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#ifdef _WIN32
#define SRL_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define SRL_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// Typedef for native C plugin initialization hook:
// SRL_PLUGIN_EXPORT bool srl_module_init(void* vm_ptr);
#else
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#define SRL_PLUGIN_EXPORT __declspec(dllexport)
#else
#define SRL_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#endif // __cplusplus

#endif // SRL_PLUGIN_H
