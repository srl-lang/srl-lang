#ifndef SRL_COMPILER_HPP
#define SRL_COMPILER_HPP

#include "ast.hpp"
#include "bytecode.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace srl {

struct Local {
    Token name;
    int depth;
};

struct CompilerContext {
    std::string name;
    Chunk chunk;
    std::vector<Local> locals;
    int scopeDepth = 0;
};

class Compiler {
public:
    Compiler();
    std::pair<Chunk, std::vector<std::shared_ptr<FunctionObject>>> compile(const std::vector<StmtPtr>& statements);
    Chunk compileFunction(const FunctionStmt* fnStmt);

private:
    CompilerContext* currentContext_ = nullptr;
    std::vector<CompilerContext> contexts_;
    std::vector<std::shared_ptr<FunctionObject>> compiledFunctions_;

    void emitByte(uint8_t byte, int line);
    void emitOp(OpCode op, int line);
    void emitBytes(uint8_t byte1, uint8_t byte2, int line);
    size_t emitJump(OpCode op, int line);
    void patchJump(size_t offset);
    void emitLoop(size_t loopStart, int line);

    void compileStmt(const Stmt* stmt);
    void compileExpr(const Expr* expr);

    void beginScope();
    void endScope(int line);
    void declareVariable(const Token& name);
    int resolveLocal(const Token& name);

    // Loop context for break/continue
    struct LoopContext {
        size_t loopStart;                   // byte offset of the condition check (for continue)
        std::vector<size_t> breakJumps;     // unpatched OP_JUMP offsets for break
        std::vector<size_t> continueJumps;  // unpatched OP_JUMP offsets for continue
    };
    std::vector<LoopContext> loopStack_;
};

} // namespace srl

#endif // SRL_COMPILER_HPP
