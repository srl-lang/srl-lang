/**
 * srl_audio_plugin.cpp - SRL Audio module as a standalone plugin DLL (v2 SDK)
 */

#include "srl_plugin_sdk.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif

static int  g_volume      = 100;
static bool g_audioLoaded = false;
static SRL_PluginCtx* g_ctx = NULL;

static std::string mciCmd(const std::string& cmd) {
#ifdef _WIN32
    char buf[128] = {0};
    MCIERROR err = mciSendStringA(cmd.c_str(), buf, sizeof(buf), NULL);
    if (err != 0) return "";
    return std::string(buf);
#else
    return "";
#endif
}

static bool audio_play_impl(const std::string& path) {
#ifdef _WIN32
    if (g_audioLoaded) {
        mciCmd("stop srl_audio");
        mciCmd("close srl_audio");
        g_audioLoaded = false;
    }
    std::string norm = path;
    for (char& c : norm) if (c == '/') c = '\\';

    if (mciCmd("open \"" + norm + "\" type mpegvideo alias srl_audio").empty()) {
        mciCmd("open \"" + path + "\" alias srl_audio");
    }
    mciCmd("set srl_audio time format milliseconds");

    int vol = (g_volume * 1000) / 100;
    std::ostringstream ss;
    ss << "setaudio srl_audio volume to " << vol;
    mciCmd(ss.str());

    mciCmd("play srl_audio");
    g_audioLoaded = true;
    return true;
#else
    std::cerr << "[srl_audio plugin] Playback not supported on this platform." << std::endl;
    return false;
#endif
}

static void fn_audio_play(int argc, const SRL_Value* args, SRL_Value* out) {
    if (argc > 0 && g_ctx->value_type(&args[0]) == SRL_TYPE_STRING) {
        bool ok = audio_play_impl(g_ctx->value_as_string(&args[0]));
        SRL_RETURN_BOOL(out, ok);
        return;
    }
    SRL_RETURN_BOOL(out, false);
}

static void fn_audio_pause(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    mciCmd("pause srl_audio");
    SRL_RETURN_BOOL(out, true);
#else
    SRL_RETURN_BOOL(out, false);
#endif
}

static void fn_audio_resume(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    mciCmd("resume srl_audio");
    SRL_RETURN_BOOL(out, true);
#else
    SRL_RETURN_BOOL(out, false);
#endif
}

static void fn_audio_stop(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    mciCmd("stop srl_audio");
    mciCmd("close srl_audio");
    g_audioLoaded = false;
    SRL_RETURN_BOOL(out, true);
#else
    SRL_RETURN_BOOL(out, false);
#endif
}

static void fn_audio_set_volume(int argc, const SRL_Value* args, SRL_Value* out) {
    if (argc > 0 && g_ctx->value_type(&args[0]) == SRL_TYPE_NUMBER) {
        int vol = (int)g_ctx->value_as_number(&args[0]);
        if (vol < 0)   vol = 0;
        if (vol > 100) vol = 100;
        g_volume = vol;
#ifdef _WIN32
        int mciVol = (vol * 1000) / 100;
        std::ostringstream ss;
        ss << "setaudio srl_audio volume to " << mciVol;
        mciCmd(ss.str());
        SRL_RETURN_BOOL(out, true);
        return;
#endif
    }
    SRL_RETURN_BOOL(out, false);
}

static void fn_audio_get_volume(int argc, const SRL_Value* args, SRL_Value* out) {
    SRL_RETURN_NUMBER(out, g_volume);
}

static void fn_audio_get_position(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    std::string res = mciCmd("status srl_audio position");
    if (!res.empty()) {
        try { SRL_RETURN_NUMBER(out, std::stod(res) / 1000.0); return; } catch (...) {}
    }
#endif
    SRL_RETURN_NUMBER(out, 0.0);
}

static void fn_audio_get_length(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    std::string res = mciCmd("status srl_audio length");
    if (!res.empty()) {
        try { SRL_RETURN_NUMBER(out, std::stod(res) / 1000.0); return; } catch (...) {}
    }
#endif
    SRL_RETURN_NUMBER(out, 0.0);
}

static void fn_audio_is_playing(int argc, const SRL_Value* args, SRL_Value* out) {
#ifdef _WIN32
    SRL_RETURN_BOOL(out, mciCmd("status srl_audio mode") == "playing");
#else
    SRL_RETURN_BOOL(out, false);
#endif
}

static void fn_audio_seek(int argc, const SRL_Value* args, SRL_Value* out) {
    if (argc > 0 && g_ctx->value_type(&args[0]) == SRL_TYPE_NUMBER) {
        double secs = g_ctx->value_as_number(&args[0]);
#ifdef _WIN32
        long long ms = (long long)(secs * 1000.0);
        std::ostringstream ss;
        ss << "seek srl_audio to " << ms;
        mciCmd(ss.str());
        if (mciCmd("status srl_audio mode") == "playing") mciCmd("play srl_audio");
        SRL_RETURN_BOOL(out, true);
        return;
#endif
    }
    SRL_RETURN_BOOL(out, false);
}

static void fn_audio_beep(int argc, const SRL_Value* args, SRL_Value* out) {
    int freq = 440, dur = 200;
    if (argc > 0 && g_ctx->value_type(&args[0]) == SRL_TYPE_NUMBER)
        freq = (int)g_ctx->value_as_number(&args[0]);
    if (argc > 1 && g_ctx->value_type(&args[1]) == SRL_TYPE_NUMBER)
        dur  = (int)g_ctx->value_as_number(&args[1]);
#ifdef _WIN32
    Beep(freq, dur);
#else
    std::cout << "\a" << std::flush;
#endif
    SRL_RETURN_NIL(out);
}

SRL_PLUGIN_EXPORT bool srl_module_init_v2(SRL_PluginCtx* ctx) {
    if (!ctx || ctx->sdk_version < 2) return false;
    g_ctx = ctx;

    SRL_DEFINE_NATIVE(ctx, "audio_play",         fn_audio_play);
    SRL_DEFINE_NATIVE(ctx, "audio_pause",        fn_audio_pause);
    SRL_DEFINE_NATIVE(ctx, "audio_resume",       fn_audio_resume);
    SRL_DEFINE_NATIVE(ctx, "audio_stop",         fn_audio_stop);
    SRL_DEFINE_NATIVE(ctx, "audio_set_volume",   fn_audio_set_volume);
    SRL_DEFINE_NATIVE(ctx, "audio_get_volume",   fn_audio_get_volume);
    SRL_DEFINE_NATIVE(ctx, "audio_get_position", fn_audio_get_position);
    SRL_DEFINE_NATIVE(ctx, "audio_get_length",   fn_audio_get_length);
    SRL_DEFINE_NATIVE(ctx, "audio_is_playing",   fn_audio_is_playing);
    SRL_DEFINE_NATIVE(ctx, "audio_seek",         fn_audio_seek);
    SRL_DEFINE_NATIVE(ctx, "audio_beep",         fn_audio_beep);

    return true;
}
