#ifndef SRL_VM_HPP
#define SRL_VM_HPP

#include "bytecode.hpp"
#include "env.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_set>

namespace srl {

enum class InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

struct CallFrame {
    std::shared_ptr<FunctionObject> function;
    const Chunk* chunk = nullptr;
    size_t ip = 0;
    size_t stackOffset = 0;
};

class VM {
public:
    VM();
    
    InterpretResult interpret(const std::string& source);
    InterpretResult interpretFile(const std::string& filepath);
    InterpretResult runFunction(const std::string& fnName);

    void defineNative(const std::string& name, NativeFn fn);
    Environment& getEnvironment() { return env_; }

private:
    Environment env_;
    std::vector<Value> stack_;
    std::vector<CallFrame> frames_;
    std::unordered_set<std::string> loadedModules_;

    void push(Value value);
    Value pop();
    Value peek(int distance = 0) const;

    InterpretResult run();

    void registerNativeFunctions();
};

} // namespace srl

#endif // SRL_VM_HPP

