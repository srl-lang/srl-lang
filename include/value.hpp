#ifndef SRL_VALUE_HPP
#define SRL_VALUE_HPP

#include <string>
#include <variant>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace srl {

struct Chunk; // Forward declare Chunk
struct Value;

enum class ValueType {
    NIL,
    BOOL,
    NUMBER,
    STRING,
    FUNCTION,
    NATIVE_FN,
    ARRAY,
    MAP
};

using NativeFn = std::function<Value(int argCount, const Value* args)>;
using ArrayPtr = std::shared_ptr<std::vector<Value>>;
using MapPtr = std::shared_ptr<std::unordered_map<std::string, Value>>;

struct FunctionObject {
    std::string name;
    int arity = 0;
    std::shared_ptr<Chunk> chunk;

    FunctionObject();
    ~FunctionObject();
};

struct Value {
    ValueType type = ValueType::NIL;
    std::variant<std::monostate, bool, double, std::string, std::shared_ptr<FunctionObject>, NativeFn, ArrayPtr, MapPtr> as;

    Value() : type(ValueType::NIL), as(std::monostate{}) {}
    Value(bool b) : type(ValueType::BOOL), as(b) {}
    Value(double n) : type(ValueType::NUMBER), as(n) {}
    Value(int n) : type(ValueType::NUMBER), as(static_cast<double>(n)) {}
    Value(std::string s) : type(ValueType::STRING), as(std::move(s)) {}
    Value(const char* s) : type(ValueType::STRING), as(std::string(s)) {}
    Value(std::shared_ptr<FunctionObject> fn) : type(ValueType::FUNCTION), as(std::move(fn)) {}
    Value(NativeFn nfn) : type(ValueType::NATIVE_FN), as(std::move(nfn)) {}
    Value(ArrayPtr arr) : type(ValueType::ARRAY), as(std::move(arr)) {}
    Value(MapPtr map) : type(ValueType::MAP), as(std::move(map)) {}

    bool isNil() const { return type == ValueType::NIL; }
    bool isBool() const { return type == ValueType::BOOL; }
    bool isNumber() const { return type == ValueType::NUMBER; }
    bool isString() const { return type == ValueType::STRING; }
    bool isFunction() const { return type == ValueType::FUNCTION; }
    bool isNativeFn() const { return type == ValueType::NATIVE_FN; }
    bool isArray() const { return type == ValueType::ARRAY; }
    bool isMap() const { return type == ValueType::MAP; }

    bool asBool() const { return std::get<bool>(as); }
    double asNumber() const { return std::get<double>(as); }
    const std::string& asString() const { return std::get<std::string>(as); }
    std::shared_ptr<FunctionObject> asFunction() const { return std::get<std::shared_ptr<FunctionObject>>(as); }
    NativeFn asNativeFn() const { return std::get<NativeFn>(as); }
    ArrayPtr asArray() const { return std::get<ArrayPtr>(as); }
    MapPtr asMap() const { return std::get<MapPtr>(as); }

    std::string toString() const;
    bool isTruthy() const;
    bool equals(const Value& other) const;
};

inline bool operator==(const Value& a, const Value& b) { return a.equals(b); }
inline bool operator!=(const Value& a, const Value& b) { return !a.equals(b); }

} // namespace srl

#endif // SRL_VALUE_HPP
