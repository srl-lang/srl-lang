#include "srl_audio.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif


namespace srl {

static int currentVolume = 100;
static bool hasAudioLoaded = false;

static std::string sendMciCommand(const std::string& cmd) {
#ifdef _WIN32
    char buffer[128] = {0};
    MCIERROR err = mciSendStringA(cmd.c_str(), buffer, sizeof(buffer), NULL);
    if (err != 0) {
        // Return empty string on MCI error
        return "";
    }
    return std::string(buffer);
#else
    return "";
#endif
}

bool Audio::play(const std::string& filepath) {
#ifdef _WIN32
    if (hasAudioLoaded) {
        stop();
    }
    sendMciCommand("close srl_audio");

    std::string normPath = filepath;
    for (char& c : normPath) {
        if (c == '/') c = '\\';
    }

    std::string openCmd = "open \"" + normPath + "\" type mpegvideo alias srl_audio";
    std::string res = sendMciCommand(openCmd);

    
    // Fallback if type mpegvideo is not supported for e.g. wav files
    if (res.empty()) {
        openCmd = "open \"" + filepath + "\" alias srl_audio";
        sendMciCommand(openCmd);
    }

    sendMciCommand("set srl_audio time format milliseconds");
    setVolume(currentVolume);

    std::string playRes = sendMciCommand("play srl_audio");
    hasAudioLoaded = true;
    return true;
#else
    std::cout << "[Audio Engine] Playback not supported on non-Windows platform yet." << std::endl;
    return false;
#endif
}

bool Audio::pause() {
#ifdef _WIN32
    sendMciCommand("pause srl_audio");
    return true;
#else
    return false;
#endif
}

bool Audio::resume() {
#ifdef _WIN32
    sendMciCommand("resume srl_audio");
    return true;
#else
    return false;
#endif
}

bool Audio::stop() {
#ifdef _WIN32
    sendMciCommand("stop srl_audio");
    sendMciCommand("close srl_audio");
    hasAudioLoaded = false;
    return true;
#else
    return false;
#endif
}

bool Audio::setVolume(int volumePercent) {
    if (volumePercent < 0) volumePercent = 0;
    if (volumePercent > 100) volumePercent = 100;
    currentVolume = volumePercent;
#ifdef _WIN32
    int mciVol = (volumePercent * 1000) / 100; // MCI volume scale 0-1000
    std::ostringstream ss;
    ss << "setaudio srl_audio volume to " << mciVol;
    sendMciCommand(ss.str());
    return true;
#else
    return false;
#endif
}

int Audio::getVolume() {
    return currentVolume;
}

double Audio::getPosition() {
#ifdef _WIN32
    std::string res = sendMciCommand("status srl_audio position");
    if (!res.empty()) {
        try {
            double ms = std::stod(res);
            return ms / 1000.0;
        } catch (...) {}
    }
#endif
    return 0.0;
}

double Audio::getLength() {
#ifdef _WIN32
    std::string res = sendMciCommand("status srl_audio length");
    if (!res.empty()) {
        try {
            double ms = std::stod(res);
            return ms / 1000.0;
        } catch (...) {}
    }
#endif
    return 0.0;
}

bool Audio::seek(double seconds) {
#ifdef _WIN32
    long long ms = static_cast<long long>(seconds * 1000.0);
    std::ostringstream ss;
    ss << "seek srl_audio to " << ms;
    sendMciCommand(ss.str());
    if (isPlaying()) {
        sendMciCommand("play srl_audio");
    }
    return true;
#else
    return false;
#endif
}

bool Audio::isPlaying() {
#ifdef _WIN32
    std::string mode = sendMciCommand("status srl_audio mode");
    return (mode == "playing");
#else
    return false;
#endif
}

void Audio::beep(int frequency, int durationMs) {
#ifdef _WIN32
    ::Beep(frequency, durationMs);
#else
    std::cout << "\a" << std::flush;
#endif
}

void Audio::registerNativeFunctions(VM& vm) {
    vm.defineNative("audio_play", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(Audio::play(args[0].asString()));
        }
        return Value(false);
    });

    vm.defineNative("audio_pause", [](int argCount, const Value* args) -> Value {
        return Value(Audio::pause());
    });

    vm.defineNative("audio_resume", [](int argCount, const Value* args) -> Value {
        return Value(Audio::resume());
    });

    vm.defineNative("audio_stop", [](int argCount, const Value* args) -> Value {
        return Value(Audio::stop());
    });

    vm.defineNative("audio_set_volume", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            return Value(Audio::setVolume(static_cast<int>(args[0].asNumber())));
        }
        return Value(false);
    });

    vm.defineNative("audio_get_volume", [](int argCount, const Value* args) -> Value {
        return Value(static_cast<double>(Audio::getVolume()));
    });

    vm.defineNative("audio_get_position", [](int argCount, const Value* args) -> Value {
        return Value(Audio::getPosition());
    });

    vm.defineNative("audio_get_length", [](int argCount, const Value* args) -> Value {
        return Value(Audio::getLength());
    });

    vm.defineNative("audio_seek", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            return Value(Audio::seek(args[0].asNumber()));
        }
        return Value(false);
    });

    vm.defineNative("audio_is_playing", [](int argCount, const Value* args) -> Value {
        return Value(Audio::isPlaying());
    });

    vm.defineNative("audio_beep", [](int argCount, const Value* args) -> Value {
        int freq = (argCount > 0 && args[0].isNumber()) ? static_cast<int>(args[0].asNumber()) : 440;
        int dur = (argCount > 1 && args[1].isNumber()) ? static_cast<int>(args[1].asNumber()) : 200;
        Audio::beep(freq, dur);
        return Value();
    });
}

} // namespace srl
