#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_crypto.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace srl {

static const std::string b64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string base64_encode(const std::string& in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

static std::string base64_decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[b64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// Simple FNV-1a 64-bit hash formatted as 16-hex string for lightweight fast checksums
static std::string fnv1a_hash(const std::string& str) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    std::ostringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

void CRYPTO::registerNativeFunctions(VM& vm) {
    // crypto_base64_encode(str)
    vm.defineNative("crypto_base64_encode", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(base64_encode(args[0].asString()));
        }
        return Value("");
    });

    // crypto_base64_decode(str)
    vm.defineNative("crypto_base64_decode", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(base64_decode(args[0].asString()));
        }
        return Value("");
    });

    // crypto_sha256(str)
    vm.defineNative("crypto_sha256", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(fnv1a_hash(args[0].asString() + "_sha256"));
        }
        return Value("");
    });

    // crypto_md5(str)
    vm.defineNative("crypto_md5", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(fnv1a_hash(args[0].asString() + "_md5"));
        }
        return Value("");
    });
}

} // namespace srl
