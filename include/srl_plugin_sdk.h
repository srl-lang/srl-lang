/**
 * srl_plugin_sdk.h - SRL Native Plugin Development Kit (v2)
 *
 * Usage:
 *   1. #include "srl_plugin_sdk.h"
 *   2. Implement native functions: void my_fn(int argc, const SRL_Value* args, SRL_Value* out)
 *   3. Use SRL_RETURN_NUMBER(out, val), SRL_RETURN_STRING(out, str), etc.
 *   4. Export srl_module_init_v2(SRL_PluginCtx* ctx) using SRL_PLUGIN_EXPORT
 *   5. Compile as dynamic library (.dll / .so)
 *
 * Example plugin:
 *
 *   #include "srl_plugin_sdk.h"
 *
 *   static SRL_PluginCtx* g_ctx = NULL;
 *
 *   static void my_add(int argc, const SRL_Value* args, SRL_Value* out) {
 *       double a = SRL_ARG_NUMBER(args, 0, 0.0);
 *       double b = SRL_ARG_NUMBER(args, 1, 0.0);
 *       SRL_RETURN_NUMBER(out, a + b);
 *   }
 *
 *   SRL_PLUGIN_EXPORT bool srl_module_init_v2(SRL_PluginCtx* ctx) {
 *       g_ctx = ctx;
 *       SRL_DEFINE_NATIVE(ctx, "my_add", my_add);
 *       return true;
 *   }
 */

#ifndef SRL_PLUGIN_SDK_H
#define SRL_PLUGIN_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* -----------------------------------------------------------------------
 * Platform export macros
 * ----------------------------------------------------------------------- */
#ifdef __cplusplus
    #ifdef _WIN32
        #define SRL_PLUGIN_EXPORT extern "C" __declspec(dllexport)
        #define SRL_PLUGIN_IMPORT extern "C" __declspec(dllimport)
    #else
        #define SRL_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
        #define SRL_PLUGIN_IMPORT extern "C"
    #endif
#else
    #ifdef _WIN32
        #define SRL_PLUGIN_EXPORT __declspec(dllexport)
        #define SRL_PLUGIN_IMPORT __declspec(dllimport)
    #else
        #define SRL_PLUGIN_EXPORT __attribute__((visibility("default")))
        #define SRL_PLUGIN_IMPORT
    #endif
#endif

/* -----------------------------------------------------------------------
 * SRL Value type tags (matches srl::ValueType in value.hpp)
 * ----------------------------------------------------------------------- */
typedef enum {
    SRL_TYPE_NIL       = 0,
    SRL_TYPE_BOOL      = 1,
    SRL_TYPE_NUMBER    = 2,
    SRL_TYPE_STRING    = 3,
    SRL_TYPE_FUNCTION  = 4,
    SRL_TYPE_NATIVE_FN = 5,
    SRL_TYPE_ARRAY     = 6,
    SRL_TYPE_MAP       = 7,
    SRL_TYPE_WEAK_REF  = 8
} SRL_ValueType;

/* -----------------------------------------------------------------------
 * Opaque SRL_Value - 128-byte aligned buffer holding srl::Value
 * ----------------------------------------------------------------------- */
struct SRL_Value {
    uint64_t _storage[10];
};

typedef struct SRL_Value SRL_Value;

/* -----------------------------------------------------------------------
 * Native function signature (ABI safe: explicit return pointer)
 * ----------------------------------------------------------------------- */
typedef void (*SRL_NativeFn)(int argCount, const SRL_Value* args, SRL_Value* out);

/* -----------------------------------------------------------------------
 * C ABI bridge function pointers
 * ----------------------------------------------------------------------- */
typedef void (*srl_define_native_fn_t)(void* vm, const char* name, SRL_NativeFn fn);
typedef void (*srl_make_nil_fn_t)(SRL_Value* out);
typedef void (*srl_make_bool_fn_t)(bool v, SRL_Value* out);
typedef void (*srl_make_number_fn_t)(double v, SRL_Value* out);
typedef void (*srl_make_string_fn_t)(const char* s, SRL_Value* out);
typedef int  (*srl_value_type_fn_t)(const SRL_Value* v);
typedef bool (*srl_value_bool_fn_t)(const SRL_Value* v);
typedef double (*srl_value_number_fn_t)(const SRL_Value* v);
typedef const char* (*srl_value_string_fn_t)(const SRL_Value* v);

/* -----------------------------------------------------------------------
 * Plugin context passed into srl_module_init_v2
 * ----------------------------------------------------------------------- */
typedef struct {
    int sdk_version;               /* SRL_PLUGIN_SDK_VERSION */
    void* vm;                      /* opaque srl::VM* */
    srl_define_native_fn_t  define_native;
    srl_make_nil_fn_t       make_nil;
    srl_make_bool_fn_t      make_bool;
    srl_make_number_fn_t    make_number;
    srl_make_string_fn_t    make_string;
    srl_value_type_fn_t     value_type;
    srl_value_bool_fn_t     value_as_bool;
    srl_value_number_fn_t   value_as_number;
    srl_value_string_fn_t   value_as_string;
} SRL_PluginCtx;

#define SRL_PLUGIN_SDK_VERSION 2

/* -----------------------------------------------------------------------
 * Convenience macros for plugin authors
 * ----------------------------------------------------------------------- */

#define SRL_DEFINE_NATIVE(ctx, name, fn) \
    (ctx)->define_native((ctx)->vm, (name), (SRL_NativeFn)(fn))

/* Read arguments safely (explicit ctx) */
#define SRL_ARG_BOOL_CTX(ctx, args, i, def) \
    ((i) < argc && (ctx)->value_type(&(args)[i]) == SRL_TYPE_BOOL \
        ? (ctx)->value_as_bool(&(args)[i]) : (def))

#define SRL_ARG_NUMBER_CTX(ctx, args, i, def) \
    ((i) < argc && (ctx)->value_type(&(args)[i]) == SRL_TYPE_NUMBER \
        ? (ctx)->value_as_number(&(args)[i]) : (def))

#define SRL_ARG_STRING_CTX(ctx, args, i, def) \
    ((i) < argc && (ctx)->value_type(&(args)[i]) == SRL_TYPE_STRING \
        ? (ctx)->value_as_string(&(args)[i]) : (def))

#define SRL_ARG_IS_NIL_CTX(ctx, args, i) \
    ((i) >= argc || (ctx)->value_type(&(args)[i]) == SRL_TYPE_NIL)

/* Convenience argument macros using global g_ctx */
#define SRL_ARG_BOOL(args, i, def)     SRL_ARG_BOOL_CTX(g_ctx, args, i, def)
#define SRL_ARG_NUMBER(args, i, def)   SRL_ARG_NUMBER_CTX(g_ctx, args, i, def)
#define SRL_ARG_STRING(args, i, def)   SRL_ARG_STRING_CTX(g_ctx, args, i, def)
#define SRL_ARG_IS_NIL(args, i)        SRL_ARG_IS_NIL_CTX(g_ctx, args, i)

/* Return value macros (explicit ctx) */
#define SRL_RETURN_NIL_CTX(ctx, out)         (ctx)->make_nil(out)
#define SRL_RETURN_BOOL_CTX(ctx, out, b)     (ctx)->make_bool((bool)(b), out)
#define SRL_RETURN_NUMBER_CTX(ctx, out, n)   (ctx)->make_number((double)(n), out)
#define SRL_RETURN_STRING_CTX(ctx, out, s)   (ctx)->make_string((const char*)(s), out)

/* Return value macros using global g_ctx */
#define SRL_RETURN_NIL(out)                  SRL_RETURN_NIL_CTX(g_ctx, out)
#define SRL_RETURN_BOOL(out, b)              SRL_RETURN_BOOL_CTX(g_ctx, out, b)
#define SRL_RETURN_NUMBER(out, n)            SRL_RETURN_NUMBER_CTX(g_ctx, out, n)
#define SRL_RETURN_STRING(out, s)            SRL_RETURN_STRING_CTX(g_ctx, out, s)

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SRL_PLUGIN_SDK_H */
