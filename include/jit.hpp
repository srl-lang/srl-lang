#ifndef SRL_JIT_HPP
#define SRL_JIT_HPP

#include "vm.hpp"
#include "bytecode.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace srl {

class JITEngine {
public:
    JITEngine();
    ~JITEngine();

    InterpretResult compileAndRun(const std::string& source);
    InterpretResult compileAndRunFile(const std::string& filepath);

private:
    void* allocateExecutableMemory(size_t size);
    void freeExecutableMemory(void* ptr, size_t size);
};

} // namespace srl

#endif // SRL_JIT_HPP
