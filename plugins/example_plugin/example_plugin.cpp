/**
 * example_plugin.cpp - SRL Plugin Template (v2 SDK)
 *
 * Demonstrates defining dynamic native functions using the v2 C ABI SDK.
 */

#include "srl_plugin_sdk.h"
#include <stdio.h>
#include <string.h>

static SRL_PluginCtx* g_ctx = NULL;

// example_add(a, b) -> number
static void fn_example_add(int argc, const SRL_Value* args, SRL_Value* out) {
    double a = SRL_ARG_NUMBER(args, 0, 0.0);
    double b = SRL_ARG_NUMBER(args, 1, 0.0);
    SRL_RETURN_NUMBER(out, a + b);
}

// example_greet(name) -> string
static void fn_example_greet(int argc, const SRL_Value* args, SRL_Value* out) {
    const char* name = SRL_ARG_STRING(args, 0, "World");
    char buf[256];
    snprintf(buf, sizeof(buf), "Hello from plugin, %s!", name);
    SRL_RETURN_STRING(out, buf);
}

// example_version() -> string
static void fn_example_version(int argc, const SRL_Value* args, SRL_Value* out) {
    SRL_RETURN_STRING(out, "example_plugin v1.0.0");
}

SRL_PLUGIN_EXPORT bool srl_module_init_v2(SRL_PluginCtx* ctx) {
    if (!ctx || ctx->sdk_version < 2) return false;
    g_ctx = ctx;

    SRL_DEFINE_NATIVE(ctx, "example_add",     fn_example_add);
    SRL_DEFINE_NATIVE(ctx, "example_greet",   fn_example_greet);
    SRL_DEFINE_NATIVE(ctx, "example_version", fn_example_version);

    return true;
}
