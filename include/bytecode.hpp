#ifndef SRL_BYTECODE_HPP
#define SRL_BYTECODE_HPP

#include "value.hpp"
#include <vector>
#include <cstdint>
#include <string>

namespace srl {

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_DEFINE_CONST,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_DUP,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NOT,
    OP_NEGATE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_RETURN
};

struct Chunk {
    std::string name;
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(line);
    }

    void writeOp(OpCode op, int line) {
        write(static_cast<uint8_t>(op), line);
    }

    size_t addConstant(Value value) {
        for (size_t i = 0; i < constants.size(); ++i) {
            if (constants[i].equals(value)) return i;
        }
        constants.push_back(std::move(value));
        return constants.size() - 1;
    }
};

} // namespace srl

#endif // SRL_BYTECODE_HPP
