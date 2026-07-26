#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_json.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cctype>

namespace srl {

class JsonParser {
public:
    JsonParser(const std::string& source) : src_(source), pos_(0) {}

    Value parse() {
        skipWhitespace();
        return parseValue();
    }

private:
    std::string src_;
    size_t pos_;

    void skipWhitespace() {
        while (pos_ < src_.length() && (src_[pos_] == ' ' || src_[pos_] == '\t' || src_[pos_] == '\n' || src_[pos_] == '\r')) {
            pos_++;
        }
    }

    char peek() const {
        return pos_ < src_.length() ? src_[pos_] : '\0';
    }

    char get() {
        return pos_ < src_.length() ? src_[pos_++] : '\0';
    }

    Value parseValue() {
        skipWhitespace();
        char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (std::isdigit(c) || c == '-') return parseNumber();
        return Value();
    }

    Value parseObject() {
        get(); // consume '{'
        auto map = std::make_shared<std::unordered_map<std::string, Value>>();
        skipWhitespace();
        if (peek() == '}') {
            get();
            return Value(map);
        }
        while (pos_ < src_.length()) {
            skipWhitespace();
            if (peek() != '"') break;
            Value keyVal = parseString();
            std::string key = keyVal.asString();
            skipWhitespace();
            if (peek() == ':') get();
            Value val = parseValue();
            (*map)[key] = val;
            skipWhitespace();
            char next = peek();
            if (next == ',') {
                get();
            } else if (next == '}') {
                get();
                break;
            } else {
                break;
            }
        }
        return Value(map);
    }

    Value parseArray() {
        get(); // consume '['
        auto arr = std::make_shared<std::vector<Value>>();
        skipWhitespace();
        if (peek() == ']') {
            get();
            return Value(arr);
        }
        while (pos_ < src_.length()) {
            Value item = parseValue();
            arr->push_back(item);
            skipWhitespace();
            char next = peek();
            if (next == ',') {
                get();
            } else if (next == ']') {
                get();
                break;
            } else {
                break;
            }
        }
        return Value(arr);
    }

    Value parseString() {
        get(); // consume '"'
        std::string res;
        while (pos_ < src_.length()) {
            char c = get();
            if (c == '"') break;
            if (c == '\\') {
                char esc = get();
                if (esc == 'n') res += '\n';
                else if (esc == 'r') res += '\r';
                else if (esc == 't') res += '\t';
                else res += esc;
            } else {
                res += c;
            }
        }
        return Value(res);
    }

    Value parseNumber() {
        size_t start = pos_;
        if (peek() == '-') get();
        while (pos_ < src_.length() && (std::isdigit(peek()) || peek() == '.' || peek() == 'e' || peek() == 'E')) {
            get();
        }
        std::string numStr = src_.substr(start, pos_ - start);
        try {
            return Value(std::stod(numStr));
        } catch (...) {
            return Value(0.0);
        }
    }

    Value parseBool() {
        if (src_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            return Value(true);
        } else if (src_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            return Value(false);
        }
        return Value(false);
    }

    Value parseNull() {
        if (src_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
        }
        return Value();
    }
};

static std::string escapeJsonString(const std::string& s) {
    std::string res = "\"";
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    res += "\"";
    return res;
}

static std::string stringifyValue(const Value& val) {
    if (val.isNil()) return "null";
    if (val.isBool()) return val.asBool() ? "true" : "false";
    if (val.isNumber()) return val.toString();
    if (val.isString()) return escapeJsonString(val.asString());
    if (val.isArray()) {
        std::string res = "[";
        auto arr = val.asArray();
        for (size_t i = 0; i < arr->size(); ++i) {
            res += stringifyValue((*arr)[i]);
            if (i + 1 < arr->size()) res += ", ";
        }
        res += "]";
        return res;
    }
    if (val.isMap()) {
        std::string res = "{";
        auto map = val.asMap();
        size_t idx = 0;
        for (const auto& [k, v] : *map) {
            res += escapeJsonString(k) + ": " + stringifyValue(v);
            if (idx + 1 < map->size()) res += ", ";
            idx++;
        }
        res += "}";
        return res;
    }
    return "null";
}


void JSON::registerNativeFunctions(VM& vm) {
    // json_parse(json_str)
    vm.defineNative("json_parse", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            JsonParser parser(args[0].asString());
            return parser.parse();
        }
        return Value();
    });

    // json_stringify(val)
    vm.defineNative("json_stringify", [](int argCount, const Value* args) -> Value {
        if (argCount > 0) {
            return Value(stringifyValue(args[0]));
        }
        return Value("null");
    });
}

} // namespace srl
