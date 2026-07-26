#include "value.hpp"
#include "bytecode.hpp"
#include <sstream>
#include <iomanip>

namespace srl {

FunctionObject::FunctionObject() : chunk(std::make_shared<Chunk>()) {}
FunctionObject::~FunctionObject() = default;

std::string Value::toString() const {
    switch (type) {
        case ValueType::NIL: return "nil";
        case ValueType::BOOL: return asBool() ? "true" : "false";
        case ValueType::NUMBER: {
            double val = asNumber();
            if (val == static_cast<long long>(val)) {
                return std::to_string(static_cast<long long>(val));
            }
            std::ostringstream ss;
            ss << std::setprecision(14) << val;
            return ss.str();
        }
        case ValueType::STRING: return asString();
        case ValueType::FUNCTION: return "<fn " + asFunction()->name + ">";
        case ValueType::NATIVE_FN: return "<native fn>";
        case ValueType::ARRAY: {
            std::string res = "[";
            auto arr = asArray();
            for (size_t i = 0; i < arr->size(); ++i) {
                res += (*arr)[i].toString();
                if (i + 1 < arr->size()) res += ", ";
            }
            res += "]";
            return res;
        }
        case ValueType::MAP: {
            std::string res = "{";
            auto map = asMap();
            size_t idx = 0;
            for (const auto& [k, v] : *map) {
                res += k + ": " + v.toString();
                if (idx + 1 < map->size()) res += ", ";
                idx++;
            }
            res += "}";
            return res;
        }
    }
    return "nil";
}

bool Value::isTruthy() const {
    switch (type) {
        case ValueType::NIL: return false;
        case ValueType::BOOL: return asBool();
        case ValueType::NUMBER: return asNumber() != 0;
        case ValueType::STRING: return !asString().empty();
        case ValueType::ARRAY: return !asArray()->empty();
        case ValueType::MAP: return !asMap()->empty();
        default: return true;
    }
}

bool Value::equals(const Value& other) const {
    if (type != other.type) return false;
    switch (type) {
        case ValueType::NIL: return true;
        case ValueType::BOOL: return asBool() == other.asBool();
        case ValueType::NUMBER: return asNumber() == other.asNumber();
        case ValueType::STRING: return asString() == other.asString();
        case ValueType::ARRAY: return asArray() == other.asArray();
        case ValueType::MAP: return asMap() == other.asMap();
        default: return false;
    }
}

} // namespace srl
