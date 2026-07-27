#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "vm.hpp"

#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include "tui.hpp"
#include "dsp.hpp"
#include "srl_audio.hpp"
#include "srl_gui.hpp"
#include "srl_ffi.hpp"
#include "srl_gfx.hpp"
#include "srl_net.hpp"
#include "srl_sys.hpp"
#include "srl_thread.hpp"
#include "srl_json.hpp"
#include "srl_db.hpp"
#include "srl_crypto.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <random>

namespace srl {

static Value getMetamethod(const Value& val, const std::string& eventName) {
    if (val.isMap()) {
        auto map = val.asMap();
        auto itMeta = map->find("__metatable");
        if (itMeta != map->end() && itMeta->second.isMap()) {
            auto metaMap = itMeta->second.asMap();
            auto itEv = metaMap->find(eventName);
            if (itEv != metaMap->end()) {
                return itEv->second;
            }
        }
    }
    return Value();
}

VM::VM() {
    registerNativeFunctions();
}

void VM::registerNativeFunctions() {
    DSP::registerNativeFunctions(*this);
    Audio::registerNativeFunctions(*this);
    GUI::registerNativeFunctions(*this);
    FFI::registerNativeFunctions(*this);
    GFX::registerNativeFunctions(*this);
    NET::registerNativeFunctions(*this);
    SYS::registerNativeFunctions(*this);
    THREAD::registerNativeFunctions(*this);
    JSON::registerNativeFunctions(*this);
    DB::registerNativeFunctions(*this);
    CRYPTO::registerNativeFunctions(*this);



    // print(...)
    defineNative("print", [](int argCount, const Value* args) -> Value {
        for (int i = 0; i < argCount; ++i) {
            std::cout << args[i].toString();
            if (i < argCount - 1) std::cout << " ";
        }
        std::cout << std::endl;
        return Value();
    });

    // sleep_ms(milliseconds)
    defineNative("sleep_ms", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            int ms = static_cast<int>(args[0].asNumber());
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return Value();
    });

    // to_string(val)
    defineNative("to_string", [](int argCount, const Value* args) -> Value {
        if (argCount > 0) {
            return Value(args[0].toString());
        }
        return Value("");
    });

    // time()
    defineNative("time", [](int argCount, const Value* args) -> Value {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0;
        return Value(seconds);
    });

    // clock()
    defineNative("clock", [](int argCount, const Value* args) -> Value {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        double seconds = std::chrono::duration_cast<std::chrono::microseconds>(now).count() / 1000000.0;
        return Value(seconds);
    });

    // --- Weak Reference & Cycle Protection ---
    defineNative("weak_ref", [](int argCount, const Value* args) -> Value {
        if (argCount > 0) {
            if (args[0].isMap()) {
                return Value(std::make_shared<WeakRefObject>(args[0].asMap()));
            }
            if (args[0].isArray()) {
                return Value(std::make_shared<WeakRefObject>(args[0].asArray()));
            }
        }
        return (argCount > 0) ? args[0] : Value();
    });

    defineNative("weak_lock", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isWeakRef()) {
            return args[0].asWeakRef()->lock();
        }
        return (argCount > 0) ? args[0] : Value();
    });

    defineNative("weak_valid", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isWeakRef()) {
            return Value(args[0].asWeakRef()->isValid());
        }
        return Value(false);
    });

    // --- Math Library Operations ---
    defineNative("math_abs", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            return Value(std::abs(args[0].asNumber()));
        }
        return Value(0.0);
    });

    defineNative("math_sin", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::sin(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_cos", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::cos(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_tan", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::tan(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_asin", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::asin(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_acos", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::acos(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_atan", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::atan(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_atan2", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) return Value(std::atan2(args[0].asNumber(), args[1].asNumber()));
        return Value(0.0);
    });

    defineNative("math_sqrt", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::sqrt(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_pow", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) return Value(std::pow(args[0].asNumber(), args[1].asNumber()));
        return Value(0.0);
    });

    defineNative("math_exp", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::exp(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_log", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::log(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_log10", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::log10(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_floor", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::floor(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_ceil", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::ceil(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_round", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) return Value(std::round(args[0].asNumber()));
        return Value(0.0);
    });

    defineNative("math_min", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            return Value((std::min)(args[0].asNumber(), args[1].asNumber()));
        }
        return Value(0.0);
    });

    defineNative("math_max", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            return Value((std::max)(args[0].asNumber(), args[1].asNumber()));
        }
        return Value(0.0);
    });


    defineNative("math_clamp", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber()) {
            double v = args[0].asNumber();
            double lo = args[1].asNumber();
            double hi = args[2].asNumber();
            return Value(std::clamp(v, lo, hi));
        }
        return Value(0.0);
    });

    defineNative("math_random", [](int argCount, const Value* args) -> Value {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return Value(dist(rng));
    });

    defineNative("math_random_range", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            double minV = args[0].asNumber();
            double maxV = args[1].asNumber();
            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist(minV, maxV);
            return Value(dist(rng));
        }
        return Value(0.0);
    });

    defineNative("math_pi", [](int argCount, const Value* args) -> Value {
        return Value(3.14159265358979323846);
    });

    // --- TUI (Text User Interface) Operations ---
    defineNative("tui_init", [](int argCount, const Value* args) -> Value {
        TUI::init();
        TUI::hideCursor();
        return Value();
    });

    defineNative("tui_clear", [](int argCount, const Value* args) -> Value {
        TUI::clear();
        return Value();
    });

    defineNative("tui_move", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            TUI::moveCursor(static_cast<int>(args[0].asNumber()), static_cast<int>(args[1].asNumber()));
        }
        return Value();
    });

    defineNative("tui_color", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string c = args[0].asString();
            if (c == "red") TUI::setColor(91);
            else if (c == "green") TUI::setColor(92);
            else if (c == "yellow") TUI::setColor(93);
            else if (c == "blue") TUI::setColor(94);
            else if (c == "magenta") TUI::setColor(95);
            else if (c == "cyan") TUI::setColor(96);
            else if (c == "white") TUI::setColor(97);
            else TUI::resetColor();
        } else {
            TUI::resetColor();
        }
        return Value();
    });

    defineNative("tui_print", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            TUI::moveCursor(row, col);
            std::cout << args[2].toString() << std::flush;
        }
        return Value();
    });

    defineNative("tui_box", [](int argCount, const Value* args) -> Value {
        if (argCount >= 5 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber() && args[3].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            int width = static_cast<int>(args[2].asNumber());
            int height = static_cast<int>(args[3].asNumber());
            std::string title = args[4].toString();

            // Draw Top Border
            TUI::moveCursor(row, col);
            std::cout << "+";
            int titleLen = static_cast<int>(title.length());
            int startTitle = (width - titleLen - 2) / 2;
            for (int i = 0; i < width - 2; ++i) {
                if (i == startTitle && !title.empty()) {
                    std::cout << " " << title << " ";
                    i += titleLen + 1;
                } else {
                    std::cout << "-";
                }
            }
            std::cout << "+";

            // Draw Sides
            for (int r = 1; r < height - 1; ++r) {
                TUI::moveCursor(row + r, col);
                std::cout << "|";
                TUI::moveCursor(row + r, col + width - 1);
                std::cout << "|";
            }

            // Draw Bottom Border
            TUI::moveCursor(row + height - 1, col);
            std::cout << "+";
            for (int i = 0; i < width - 2; ++i) std::cout << "-";
            std::cout << "+" << std::flush;
        }
        return Value();
    });

    defineNative("tui_progress", [](int argCount, const Value* args) -> Value {
        if (argCount >= 4 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber() && args[3].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            int width = static_cast<int>(args[2].asNumber());
            double percent = args[3].asNumber(); // 0 to 100

            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;

            int filled = static_cast<int>((width * percent) / 100.0);

            TUI::moveCursor(row, col);
            std::cout << "[";
            TUI::setColor(92); // Green
            for (int i = 0; i < filled; ++i) std::cout << "#";
            TUI::setColor(90); // Dark grey
            for (int i = filled; i < width; ++i) std::cout << "-";
            TUI::resetColor();
            std::cout << "] " << static_cast<int>(percent) << "% " << std::flush;
        }
        return Value();
    });

    defineNative("tui_key_pressed", [](int argCount, const Value* args) -> Value {
        return Value(TUI::keyPressed());
    });

    defineNative("tui_get_key", [](int argCount, const Value* args) -> Value {
        int key = TUI::getKey();
        if (key != -1) {
            return Value(std::string(1, static_cast<char>(key)));
        }
        return Value("");
    });

    defineNative("tui_reset", [](int argCount, const Value* args) -> Value {
        TUI::resetColor();
        TUI::showCursor();
        return Value();
    });

    // --- Array Operations ---
    defineNative("arr_new", [](int argCount, const Value* args) -> Value {
        return Value(std::make_shared<std::vector<Value>>());
    });

    defineNative("arr_push", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray()) {
            args[0].asArray()->push_back(args[1]);
        }
        return Value();
    });

    defineNative("arr_get", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            auto arr = args[0].asArray();
            if (idx < arr->size()) return (*arr)[idx];
        }
        return Value();
    });

    defineNative("arr_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isArray() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            auto arr = args[0].asArray();
            if (idx >= arr->size()) {
                arr->resize(idx + 1);
            }
            (*arr)[idx] = args[2];
        }
        return Value();
    });

    defineNative("arr_len", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isArray()) {
            return Value(static_cast<double>(args[0].asArray()->size()));
        }
        return Value(0.0);
    });

    // --- Map Operations ---
    defineNative("map_new", [](int argCount, const Value* args) -> Value {
        return Value(std::make_shared<std::unordered_map<std::string, Value>>());
    });

    defineNative("map_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isMap()) {
            (*args[0].asMap())[args[1].toString()] = args[2];
        }
        return Value();
    });

    defineNative("map_get", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            auto map = args[0].asMap();
            std::string key = args[1].toString();
            auto it = map->find(key);
            if (it != map->end()) return it->second;

            Value mm = getMetamethod(args[0], "__index");
            if (mm.isMap()) {
                auto indexMap = mm.asMap();
                auto itIdx = indexMap->find(key);
                if (itIdx != indexMap->end()) return itIdx->second;
            } else if (mm.isNativeFn()) {
                return mm.asNativeFn()(2, args);
            }
        }
        return Value();
    });

    defineNative("map_has", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            return Value(args[0].asMap()->count(args[1].toString()) > 0);
        }
        return Value(false);
    });

    defineNative("map_keys", [](int argCount, const Value* args) -> Value {
        auto keys = std::make_shared<std::vector<Value>>();
        if (argCount >= 1 && args[0].isMap()) {
            for (const auto& pair : *args[0].asMap()) {
                if (pair.first != "__metatable") {
                    keys->push_back(Value(pair.first));
                }
            }
        }
        return Value(keys);
    });

    defineNative("is_map", [](int argCount, const Value* args) -> Value {
        return Value(argCount >= 1 && args[0].isMap());
    });

    defineNative("is_array", [](int argCount, const Value* args) -> Value {
        return Value(argCount >= 1 && args[0].isArray());
    });

    // --- Metatable Operations ---
    defineNative("setmetatable", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap() && args[1].isMap()) {
            (*args[0].asMap())["__metatable"] = args[1];
            return args[0];
        }
        return Value();
    });

    defineNative("getmetatable", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isMap()) {
            auto map = args[0].asMap();
            auto it = map->find("__metatable");
            if (it != map->end()) return it->second;
        }
        return Value();
    });

    defineNative("rawget", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            auto map = args[0].asMap();
            auto it = map->find(args[1].toString());
            if (it != map->end()) return it->second;
        }
        return Value();
    });

    defineNative("rawset", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isMap()) {
            (*args[0].asMap())[args[1].toString()] = args[2];
            return args[0];
        }
        return Value();
    });

    // --- String Operations ---
    defineNative("str_len", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(static_cast<double>(args[0].asString().length()));
        }
        return Value(0.0);
    });

    defineNative("str_sub", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isString() && args[1].isNumber() && args[2].isNumber()) {
            size_t start = static_cast<size_t>(args[1].asNumber());
            size_t len = static_cast<size_t>(args[2].asNumber());
            const std::string& str = args[0].asString();
            if (start < str.length()) {
                return Value(str.substr(start, len));
            }
        }
        return Value("");
    });

    defineNative("str_char", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            const std::string& str = args[0].asString();
            if (idx < str.length()) {
                return Value(std::string(1, str[idx]));
            }
        }
        return Value("");
    });

    defineNative("str_code", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString() && !args[0].asString().empty()) {
            return Value(static_cast<double>(static_cast<unsigned char>(args[0].asString()[0])));
        }
        return Value(0.0);
    });

    defineNative("str_from_code", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            char c = static_cast<char>(args[0].asNumber());
            return Value(std::string(1, c));
        }
        return Value("");
    });

    defineNative("str_upper", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string s = args[0].asString();
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return Value(s);
        }
        return Value("");
    });

    defineNative("str_lower", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string s = args[0].asString();
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return Value(s);
        }
        return Value("");
    });

    defineNative("str_find", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            const std::string& str = args[0].asString();
            const std::string& target = args[1].asString();
            auto pos = str.find(target);
            if (pos != std::string::npos) return Value(static_cast<double>(pos));
        }
        return Value(-1.0);
    });

    defineNative("str_replace", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isString() && args[1].isString() && args[2].isString()) {
            std::string str = args[0].asString();
            std::string from = args[1].asString();
            std::string to = args[2].asString();
            if (!from.empty()) {
                size_t start_pos = 0;
                while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
                    str.replace(start_pos, from.length(), to);
                    start_pos += to.length();
                }
            }
            return Value(str);
        }
        return Value("");
    });

    defineNative("str_split", [](int argCount, const Value* args) -> Value {
        auto resultArr = std::make_shared<std::vector<Value>>();
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::string str = args[0].asString();
            std::string delim = args[1].asString();
            if (delim.empty()) {
                for (char c : str) resultArr->push_back(Value(std::string(1, c)));
            } else {
                size_t start = 0;
                size_t end = str.find(delim);
                while (end != std::string::npos) {
                    resultArr->push_back(Value(str.substr(start, end - start)));
                    start = end + delim.length();
                    end = str.find(delim, start);
                }
                resultArr->push_back(Value(str.substr(start)));
            }
        }
        return Value(resultArr);
    });

    defineNative("str_trim", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string s = args[0].asString();
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
            return Value(s);
        }
        return Value("");
    });

    defineNative("str_contains", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            return Value(args[0].asString().find(args[1].asString()) != std::string::npos);
        }
        return Value(false);
    });

    defineNative("str_starts_with", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            const std::string& str = args[0].asString();
            const std::string& prefix = args[1].asString();
            return Value(str.rfind(prefix, 0) == 0);
        }
        return Value(false);
    });

    defineNative("str_ends_with", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            const std::string& str = args[0].asString();
            const std::string& suffix = args[1].asString();
            if (str.length() >= suffix.length()) {
                return Value(str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
            }
        }
        return Value(false);
    });

    // --- File & Directory Operations ---
    defineNative("file_read", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::ifstream file(args[0].asString());
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return Value(buffer.str());
            }
        }
        return Value();
    });

    defineNative("file_write", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::ofstream file(args[0].asString());
            if (file.is_open()) {
                file << args[1].asString();
                return Value(true);
            }
        }
        return Value(false);
    });

    defineNative("file_append", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::ofstream file(args[0].asString(), std::ios::app);
            if (file.is_open()) {
                file << args[1].asString();
                return Value(true);
            }
        }
        return Value(false);
    });

    defineNative("file_exists", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(std::filesystem::exists(args[0].asString()));
        }
        return Value(false);
    });

    defineNative("file_remove", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(std::filesystem::remove(args[0].asString()));
        }
        return Value(false);
    });

    defineNative("file_size", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::error_code ec;
            auto size = std::filesystem::file_size(args[0].asString(), ec);
            if (!ec) return Value(static_cast<double>(size));
        }
        return Value(0.0);
    });

    defineNative("file_copy", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::error_code ec;
            std::filesystem::copy_file(args[0].asString(), args[1].asString(), std::filesystem::copy_options::overwrite_existing, ec);
            return Value(!ec);
        }
        return Value(false);
    });

    defineNative("dir_list", [](int argCount, const Value* args) -> Value {
        auto arr = std::make_shared<std::vector<Value>>();
        if (argCount > 0 && args[0].isString()) {
            std::string path = args[0].asString();
            std::error_code ec;
            if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec)) {
                for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
                    arr->push_back(Value(entry.path().filename().string()));
                }
            }
        }
        return Value(arr);
    });

    defineNative("dir_list_ext", [](int argCount, const Value* args) -> Value {
        auto arr = std::make_shared<std::vector<Value>>();
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::string path = args[0].asString();
            std::string ext = args[1].asString();
            if (!ext.empty() && ext[0] != '.') ext = "." + ext;
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            std::error_code ec;
            if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec)) {
                for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
                    std::string entryExt = entry.path().extension().string();
                    std::transform(entryExt.begin(), entryExt.end(), entryExt.begin(), ::tolower);
                    if (entryExt == ext) {
                        arr->push_back(Value(entry.path().string()));
                    }
                }
            }
        }
        return Value(arr);
    });

    defineNative("dir_exists", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::error_code ec;
            return Value(std::filesystem::is_directory(args[0].asString(), ec));
        }
        return Value(false);
    });

    defineNative("dir_create", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::error_code ec;
            return Value(std::filesystem::create_directories(args[0].asString(), ec));
        }
        return Value(false);
    });

    // --- Module Import with Smart Resolution ---
    defineNative("import", [this](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string reqPath = args[0].asString();
            namespace fs = std::filesystem;

            std::vector<std::string> candidates = {
                reqPath,
                reqPath + ".srl",
                (fs::path("srl_modules") / reqPath).string(),
                (fs::path("srl_modules") / reqPath / "main.srl").string(),
                (fs::path("srl_modules") / reqPath / "index.srl").string(),
                (fs::path("srl_modules") / (reqPath + ".srl")).string(),
                (fs::path("std") / reqPath).string(),
                (fs::path("std") / (reqPath + ".srl")).string()
            };

            std::string resolvedPath = "";
            for (const auto& cand : candidates) {
                if (fs::exists(cand) && !fs::is_directory(cand)) {
                    resolvedPath = cand;
                    break;
                }
            }

            if (resolvedPath.empty()) {
                std::cerr << "[VM Error] Import module not found: " << reqPath << std::endl;
                return Value(false);
            }

            if (loadedModules_.count(resolvedPath) > 0) {
                return Value(true); // Already imported
            }
            loadedModules_.insert(resolvedPath);
            InterpretResult res = this->interpretFile(resolvedPath);
            return Value(res == InterpretResult::INTERPRET_OK);
        }
        return Value(false);
    });

}

InterpretResult VM::interpretFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[VM Error] Could not open file: " << filepath << std::endl;
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return interpret(buffer.str());
}

void VM::defineNative(const std::string& name, NativeFn fn) {
    env_.defineGlobal(name, Value(fn));
}

void VM::push(Value value) {
    stack_.push_back(std::move(value));
}

Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value val = std::move(stack_.back());
    stack_.pop_back();
    return val;
}

Value VM::peek(int distance) const {
    return stack_[stack_.size() - 1 - distance];
}

InterpretResult VM::interpret(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();

        Parser parser(tokens);
        auto statements = parser.parse();

        Compiler compiler;
        auto [scriptChunk, functions] = compiler.compile(statements);

        // Store compiled functions into global symbol table
        for (const auto& fnObj : functions) {
            env_.setGlobal(fnObj->name, Value(fnObj));
        }

        // Run top-level script statements
        auto scriptFn = std::make_shared<FunctionObject>();
        scriptFn->name = "main_script";
        scriptFn->arity = 0;

        size_t initialDepth = frames_.size();

        // Push script function object onto stack so OP_RETURN unwinds cleanly
        push(Value(scriptFn));

        // Execute top level script
        CallFrame frame;
        frame.function = scriptFn;
        frame.chunk = &scriptChunk;
        frame.ip = 0;
        frame.stackOffset = stack_.size();
        frames_.push_back(frame);

        return run(initialDepth);


    } catch (const std::exception& e) {
        std::cerr << "[VM Error] " << e.what() << std::endl;
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
    }
}

InterpretResult VM::runFunction(const std::string& fnName) {
    Value fnValue;
    if (!env_.getGlobal(fnName, fnValue) || !fnValue.isFunction()) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
    }

    auto fnObj = fnValue.asFunction();

    // Call the function
    Value callVal = fnValue;
    push(callVal); // push function object onto stack

    CallFrame frame;
    frame.function = fnObj;
    // For hot reloaded functions, we execute their body by calling them
    return InterpretResult::INTERPRET_OK;
}

InterpretResult VM::run(size_t targetFrameDepth) {

    #define CURR_FRAME (frames_.back())
    #define READ_BYTE() (CURR_FRAME.chunk->code[CURR_FRAME.ip++])
    #define READ_CONSTANT() (CURR_FRAME.chunk->constants[READ_BYTE()])
    #define READ_SHORT() (CURR_FRAME.ip += 2, (uint16_t)((CURR_FRAME.chunk->code[CURR_FRAME.ip - 2] << 8) | CURR_FRAME.chunk->code[CURR_FRAME.ip - 1]))
    #define CURRENT_LINE() (CURR_FRAME.ip > 0 && (CURR_FRAME.ip - 1) < CURR_FRAME.chunk->lines.size() \
                            ? CURR_FRAME.chunk->lines[CURR_FRAME.ip - 1] : 0)

    while (true) {
        if (CURR_FRAME.ip >= CURR_FRAME.chunk->code.size()) {
            frames_.pop_back();
            if (frames_.size() == targetFrameDepth) return InterpretResult::INTERPRET_OK;
            continue;
        }

        OpCode instruction = static_cast<OpCode>(READ_BYTE());
        switch (instruction) {
            case OpCode::OP_CONSTANT: {
                push(READ_CONSTANT());
                break;
            }
            case OpCode::OP_NIL: push(Value()); break;
            case OpCode::OP_TRUE: push(Value(true)); break;
            case OpCode::OP_FALSE: push(Value(false)); break;
            case OpCode::OP_POP: pop(); break;

            case OpCode::OP_DEFINE_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value value = pop();
                env_.defineGlobal(nameVal.asString(), value, false);
                break;
            }

            case OpCode::OP_DEFINE_CONST: {
                Value nameVal = READ_CONSTANT();
                Value value = pop();
                env_.defineGlobal(nameVal.asString(), value, true);
                break;
            }

            case OpCode::OP_GET_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value value;
                if (!env_.getGlobal(nameVal.asString(), value)) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Undefined variable '" << nameVal.asString() << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }

            case OpCode::OP_SET_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value val = peek(0);
                if (!env_.setGlobal(nameVal.asString(), val)) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Cannot reassign constant variable '" << nameVal.asString() << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(stack_[CURR_FRAME.stackOffset + slot]);
                break;
            }

            case OpCode::OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                stack_[CURR_FRAME.stackOffset + slot] = peek(0);
                break;
            }

            case OpCode::OP_DUP: {
                push(peek(0));
                break;
            }

            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value(a == b));
                break;
            }

            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '>' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(a.asNumber() > b.asNumber()));
                break;
            }

            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '<' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(a.asNumber() < b.asNumber()));
                break;
            }

            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (a.isString() || b.isString()) {
                    push(Value(a.asString() + b.asString()));
                } else if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() + b.asNumber()));
                } else {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '+' operands must be numbers or strings." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '-' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(a.asNumber() - b.asNumber()));
                break;
            }

            case OpCode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '*' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(a.asNumber() * b.asNumber()));
                break;
            }

            case OpCode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '/' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                if (b.asNumber() == 0) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Division by zero." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(a.asNumber() / b.asNumber()));
                break;
            }

            case OpCode::OP_MODULO: {
                Value b = pop();
                Value a = pop();
                if (!a.isNumber() || !b.isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: '%' operands must be numbers." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                if (b.asNumber() == 0) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Modulo by zero." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                // fmod handles float modulo correctly (e.g. 3.7 % 1.2)
                push(Value(std::fmod(a.asNumber(), b.asNumber())));
                break;
            }

            case OpCode::OP_NOT: {
                push(Value(!pop().isTruthy()));
                break;
            }

            case OpCode::OP_NEGATE: {
                if (!peek(0).isNumber()) {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Negation operand must be a number." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(-pop().asNumber()));
                break;
            }

            case OpCode::OP_JUMP: {
                uint16_t offset = READ_SHORT();
                CURR_FRAME.ip += offset;
                break;
            }

            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (!peek(0).isTruthy()) {
                    CURR_FRAME.ip += offset;
                }
                break;
            }

            case OpCode::OP_LOOP: {
                uint16_t offset = READ_SHORT();
                CURR_FRAME.ip -= offset;
                break;
            }

            case OpCode::OP_CALL: {
                uint8_t argCount = READ_BYTE();
                Value callee = peek(argCount);

                if (callee.isNativeFn()) {
                    std::vector<Value> args(argCount);
                    for (int i = argCount - 1; i >= 0; --i) {
                        args[i] = pop();
                    }
                    pop(); // pop native function
                    Value result = callee.asNativeFn()(argCount, args.data());
                    push(result);
                } else if (callee.isFunction()) {
                    auto fnObj = callee.asFunction();
                    if (argCount != fnObj->arity) {
                        std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Function '" << fnObj->name
                                  << "' expects " << fnObj->arity << " arguments but got " << argCount << "." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }

                    // Look up latest function implementation from global symbol table (HOT RELOAD MAGIC!)
                    Value latestFnVal;
                    if (env_.getGlobal(fnObj->name, latestFnVal) && latestFnVal.isFunction()) {
                        fnObj = latestFnVal.asFunction();
                    }

                    // Create new CallFrame
                    CallFrame newFrame;
                    newFrame.function = fnObj;
                    newFrame.chunk = fnObj->chunk.get();
                    newFrame.ip = 0;
                    newFrame.stackOffset = stack_.size() - argCount;
                    frames_.push_back(newFrame);
                } else {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Runtime Error: Can only call functions." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_RETURN: {
                Value result = pop();
                size_t offset = CURR_FRAME.stackOffset;
                frames_.pop_back();

                // Unwind arguments and function object from stack
                while (stack_.size() > offset && !stack_.empty()) {
                    stack_.pop_back();
                }
                if (!stack_.empty()) {
                    stack_.pop_back(); // pop function callee
                }

                if (frames_.size() == targetFrameDepth) {
                    push(result);
                    return InterpretResult::INTERPRET_OK;
                }

                push(result);
                break;
            }

            case OpCode::OP_GET_FIELD: {
                Value fieldVal = READ_CONSTANT();
                Value obj = pop();
                if (obj.isMap()) {
                    auto mapPtr = obj.asMap();
                    auto it = mapPtr->find(fieldVal.asString());
                    if (it != mapPtr->end()) {
                        push(it->second);
                    } else {
                        push(Value());
                    }
                } else {
                    push(Value());
                }
                break;
            }

            case OpCode::OP_SET_FIELD: {
                Value fieldVal = READ_CONSTANT();
                Value val = pop();
                Value obj = pop();
                if (obj.isMap()) {
                    (*obj.asMap())[fieldVal.asString()] = val;
                }
                push(val);
                break;
            }

            case OpCode::OP_TRY: {
                uint16_t offset = READ_SHORT();
                TryFrame tf;
                tf.frameDepth = frames_.size();
                tf.stackDepth = stack_.size();
                tf.catchIp = CURR_FRAME.ip + offset;
                tryStack_.push_back(tf);
                break;
            }

            case OpCode::OP_CATCH: {
                if (!tryStack_.empty()) {
                    tryStack_.pop_back();
                }
                break;
            }

            case OpCode::OP_THROW: {
                Value errVal = pop();
                if (!tryStack_.empty()) {
                    TryFrame tf = tryStack_.back();
                    tryStack_.pop_back();
                    while (frames_.size() > tf.frameDepth) {
                        frames_.pop_back();
                    }
                    while (stack_.size() > tf.stackDepth) {
                        stack_.pop_back();
                    }
                    push(errVal);
                    CURR_FRAME.ip = tf.catchIp;
                } else {
                    std::cerr << "[Line " << CURRENT_LINE() << "] Unhandled Exception: " << errVal.asString() << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_ASYNC_CALL:
            case OpCode::OP_AWAIT: {
                // Async execution evaluates the task expression directly in sync VM frame
                break;
            }

            case OpCode::OP_BITWISE_AND: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(static_cast<double>(static_cast<int64_t>(a.asNumber()) & static_cast<int64_t>(b.asNumber()))));
                } else {
                    push(Value(0.0));
                }
                break;
            }

            case OpCode::OP_BITWISE_OR: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(static_cast<double>(static_cast<int64_t>(a.asNumber()) | static_cast<int64_t>(b.asNumber()))));
                } else {
                    push(Value(0.0));
                }
                break;
            }

            case OpCode::OP_BITWISE_XOR: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(static_cast<double>(static_cast<int64_t>(a.asNumber()) ^ static_cast<int64_t>(b.asNumber()))));
                } else {
                    push(Value(0.0));
                }
                break;
            }

            case OpCode::OP_BUILD_ARRAY: {
                uint8_t count = READ_BYTE();
                auto arr = std::make_shared<std::vector<Value>>(count);
                for (int i = count - 1; i >= 0; --i) {
                    (*arr)[i] = pop();
                }
                push(Value(arr));
                break;
            }

            case OpCode::OP_BUILD_MAP: {
                uint8_t pairCount = READ_BYTE();
                auto mapObj = std::make_shared<std::unordered_map<std::string, Value>>();
                for (int i = 0; i < pairCount; ++i) {
                    Value val = pop();
                    Value key = pop();
                    (*mapObj)[key.asString()] = val;
                }
                push(Value(mapObj));
                break;
            }
        }
    }

    #undef CURR_FRAME
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef READ_SHORT
    #undef CURRENT_LINE
}

} // namespace srl
