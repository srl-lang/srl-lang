#include "compiler.hpp"
#include <iostream>
#include <stdexcept>

namespace srl {

Compiler::Compiler() {}

std::pair<Chunk, std::vector<std::shared_ptr<FunctionObject>>> Compiler::compile(const std::vector<StmtPtr>& statements) {
    contexts_.clear();
    compiledFunctions_.clear();

    CompilerContext scriptContext;
    scriptContext.name = "script_main";
    contexts_.push_back(std::move(scriptContext));
    currentContext_ = &contexts_.back();

    for (const auto& stmt : statements) {
        compileStmt(stmt.get());
    }

    emitOp(OpCode::OP_NIL, 0);
    emitOp(OpCode::OP_RETURN, 0);

    Chunk scriptChunk = currentContext_->chunk;
    return {scriptChunk, compiledFunctions_};
}

Chunk Compiler::compileFunction(const FunctionStmt* fnStmt) {
    CompilerContext fnContext;
    fnContext.name = fnStmt->name.lexeme;
    currentContext_ = &fnContext;

    beginScope();
    for (const auto& param : fnStmt->params) {
        declareVariable(param);
    }

    for (const auto& stmt : fnStmt->body) {
        compileStmt(stmt.get());
    }

    // Default return nil if no return expression hit
    emitOp(OpCode::OP_NIL, fnStmt->name.line);
    emitOp(OpCode::OP_RETURN, fnStmt->name.line);

    return fnContext.chunk;
}

void Compiler::emitByte(uint8_t byte, int line) {
    currentContext_->chunk.write(byte, line);
}

void Compiler::emitOp(OpCode op, int line) {
    currentContext_->chunk.writeOp(op, line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2, int line) {
    emitByte(byte1, line);
    emitByte(byte2, line);
}

size_t Compiler::emitJump(OpCode op, int line) {
    emitOp(op, line);
    emitByte(0xff, line);
    emitByte(0xff, line);
    return currentContext_->chunk.code.size() - 2;
}

void Compiler::patchJump(size_t offset) {
    size_t jump = currentContext_->chunk.code.size() - offset - 2;
    if (jump > UINT16_MAX) {
        throw std::runtime_error("Too much code to jump over.");
    }
    currentContext_->chunk.code[offset] = (jump >> 8) & 0xff;
    currentContext_->chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(size_t loopStart, int line) {
    emitOp(OpCode::OP_LOOP, line);
    size_t jump = currentContext_->chunk.code.size() - loopStart + 2;
    if (jump > UINT16_MAX) {
        throw std::runtime_error("Loop body too large.");
    }
    emitByte((jump >> 8) & 0xff, line);
    emitByte(jump & 0xff, line);
}

void Compiler::beginScope() {
    currentContext_->scopeDepth++;
}

void Compiler::endScope(int line) {
    currentContext_->scopeDepth--;
    while (!currentContext_->locals.empty() && 
           currentContext_->locals.back().depth > currentContext_->scopeDepth) {
        emitOp(OpCode::OP_POP, line);
        currentContext_->locals.pop_back();
    }
}

void Compiler::declareVariable(const Token& name) {
    if (currentContext_->scopeDepth == 0) return;
    for (auto it = currentContext_->locals.rbegin(); it != currentContext_->locals.rend(); ++it) {
        if (it->depth != -1 && it->depth < currentContext_->scopeDepth) break;
        if (it->name.lexeme == name.lexeme) {
            throw std::runtime_error("Variable with name '" + name.lexeme + "' already declared in this scope.");
        }
    }
    currentContext_->locals.push_back({name, currentContext_->scopeDepth});
}

int Compiler::resolveLocal(const Token& name) {
    for (int i = static_cast<int>(currentContext_->locals.size()) - 1; i >= 0; --i) {
        if (currentContext_->locals[i].name.lexeme == name.lexeme) {
            return i;
        }
    }
    return -1;
}

void Compiler::compileStmt(const Stmt* stmt) {
    int line = 0;
    switch (stmt->getType()) {
        case ASTNodeType::EXPRESSION_STMT: {
            auto exprStmt = static_cast<const ExpressionStmt*>(stmt);
            compileExpr(exprStmt->expression.get());
            emitOp(OpCode::OP_POP, line);
            break;
        }

        case ASTNodeType::VAR_STMT: {
            auto varStmt = static_cast<const VarStmt*>(stmt);
            declareVariable(varStmt->name);
            if (varStmt->initializer) {
                compileExpr(varStmt->initializer.get());
            } else {
                emitOp(OpCode::OP_NIL, varStmt->name.line);
            }

            if (currentContext_->scopeDepth == 0) {
                size_t globalIdx = currentContext_->chunk.addConstant(Value(varStmt->name.lexeme));
                emitOp(OpCode::OP_DEFINE_GLOBAL, varStmt->name.line);
                emitByte(static_cast<uint8_t>(globalIdx), varStmt->name.line);
            }
            break;
        }

        case ASTNodeType::FUNCTION_STMT: {
            auto fnStmt = static_cast<const FunctionStmt*>(stmt);
            auto prevContext = currentContext_;

            Compiler fnCompiler;
            Chunk fnChunk = fnCompiler.compileFunction(fnStmt);

            currentContext_ = prevContext;

            auto fnObj = std::make_shared<FunctionObject>();
            fnObj->name = fnStmt->name.lexeme;
            fnObj->arity = static_cast<int>(fnStmt->params.size());
            fnObj->chunk = std::make_shared<Chunk>(std::move(fnChunk));
            
            // We store function chunk in function object
            size_t constIdx = currentContext_->chunk.addConstant(Value(fnObj));
            
            compiledFunctions_.push_back(fnObj);
            // also append compiled function chunks to function object
            
            size_t globalIdx = currentContext_->chunk.addConstant(Value(fnStmt->name.lexeme));
            
            emitOp(OpCode::OP_CONSTANT, fnStmt->name.line);
            emitByte(static_cast<uint8_t>(constIdx), fnStmt->name.line);

            emitOp(OpCode::OP_DEFINE_GLOBAL, fnStmt->name.line);
            emitByte(static_cast<uint8_t>(globalIdx), fnStmt->name.line);
            break;
        }

        case ASTNodeType::STRUCT_STMT: {
            auto structStmt = static_cast<const StructStmt*>(stmt);

            std::vector<StmtPtr> bodyStmts;

            Token objToken(TokenType::IDENTIFIER, "__obj", structStmt->name.line, structStmt->name.column);
            Token mapNewToken(TokenType::IDENTIFIER, "map_new", structStmt->name.line, structStmt->name.column);
            Token parenToken(TokenType::RIGHT_PAREN, ")", structStmt->name.line, structStmt->name.column);

            auto callMapNew = std::make_unique<CallExpr>(
                std::make_unique<VariableExpr>(mapNewToken),
                parenToken,
                std::vector<ExprPtr>()
            );
            bodyStmts.push_back(std::make_unique<VarStmt>(objToken, std::move(callMapNew)));

            for (const auto& field : structStmt->fields) {
                Token mapSetToken(TokenType::IDENTIFIER, "map_set", structStmt->name.line, structStmt->name.column);
                std::vector<ExprPtr> args;
                args.push_back(std::make_unique<VariableExpr>(objToken));
                args.push_back(std::make_unique<LiteralExpr>(Value(field.lexeme)));
                args.push_back(std::make_unique<VariableExpr>(field));

                auto callMapSet = std::make_unique<CallExpr>(
                    std::make_unique<VariableExpr>(mapSetToken),
                    parenToken,
                    std::move(args)
                );
                bodyStmts.push_back(std::make_unique<ExpressionStmt>(std::move(callMapSet)));
            }

            Token retToken(TokenType::KEYWORD_RETURN, "return", structStmt->name.line, structStmt->name.column);
            bodyStmts.push_back(std::make_unique<ReturnStmt>(retToken, std::make_unique<VariableExpr>(objToken)));

            FunctionStmt ctorStmt(structStmt->name, structStmt->fields, std::move(bodyStmts));

            auto prevContext = currentContext_;
            Compiler fnCompiler;
            Chunk fnChunk = fnCompiler.compileFunction(&ctorStmt);
            currentContext_ = prevContext;

            auto fnObj = std::make_shared<FunctionObject>();
            fnObj->name = structStmt->name.lexeme;
            fnObj->arity = static_cast<int>(structStmt->fields.size());
            fnObj->chunk = std::make_shared<Chunk>(std::move(fnChunk));

            size_t constIdx = currentContext_->chunk.addConstant(Value(fnObj));
            compiledFunctions_.push_back(fnObj);

            size_t globalIdx = currentContext_->chunk.addConstant(Value(structStmt->name.lexeme));

            emitOp(OpCode::OP_CONSTANT, structStmt->name.line);
            emitByte(static_cast<uint8_t>(constIdx), structStmt->name.line);

            emitOp(OpCode::OP_DEFINE_GLOBAL, structStmt->name.line);
            emitByte(static_cast<uint8_t>(globalIdx), structStmt->name.line);
            break;
        }

        case ASTNodeType::BLOCK_STMT: {
            auto blockStmt = static_cast<const BlockStmt*>(stmt);
            beginScope();
            for (const auto& s : blockStmt->statements) {
                compileStmt(s.get());
            }
            endScope(line);
            break;
        }

        case ASTNodeType::IF_STMT: {
            auto ifStmt = static_cast<const IfStmt*>(stmt);
            compileExpr(ifStmt->condition.get());

            size_t thenJump = emitJump(OpCode::OP_JUMP_IF_FALSE, line);
            emitOp(OpCode::OP_POP, line); // Pop condition

            compileStmt(ifStmt->thenBranch.get());

            size_t elseJump = emitJump(OpCode::OP_JUMP, line);
            patchJump(thenJump);
            emitOp(OpCode::OP_POP, line); // Pop condition

            if (ifStmt->elseBranch) {
                compileStmt(ifStmt->elseBranch.get());
            }
            patchJump(elseJump);
            break;
        }

        case ASTNodeType::WHILE_STMT: {
            auto whileStmt = static_cast<const WhileStmt*>(stmt);
            size_t loopStart = currentContext_->chunk.code.size();
            compileExpr(whileStmt->condition.get());

            size_t exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE, line);
            emitOp(OpCode::OP_POP, line); // Pop condition

            compileStmt(whileStmt->body.get());
            emitLoop(loopStart, line);

            patchJump(exitJump);
            emitOp(OpCode::OP_POP, line); // Pop condition
            break;
        }

        case ASTNodeType::RETURN_STMT: {
            auto retStmt = static_cast<const ReturnStmt*>(stmt);
            if (retStmt->value) {
                compileExpr(retStmt->value.get());
            } else {
                emitOp(OpCode::OP_NIL, retStmt->keyword.line);
            }
            emitOp(OpCode::OP_RETURN, retStmt->keyword.line);
            break;
        }

        case ASTNodeType::FOR_STMT: {
            auto forStmt = static_cast<const ForStmt*>(stmt);
            beginScope();

            // 1. Initializer
            if (forStmt->initializer) {
                compileStmt(forStmt->initializer.get());
            }

            // 2. Condition check
            size_t loopStart = currentContext_->chunk.code.size();
            size_t exitJump = SIZE_MAX;

            if (forStmt->condition) {
                compileExpr(forStmt->condition.get());
                exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE, 0);
                emitOp(OpCode::OP_POP, 0); // pop condition
            }

            // 3. Body
            compileStmt(forStmt->body.get());

            // 4. Increment
            if (forStmt->increment) {
                compileExpr(forStmt->increment.get());
                emitOp(OpCode::OP_POP, 0); // pop increment result
            }

            // 5. Loop back
            emitLoop(loopStart, 0);

            if (exitJump != SIZE_MAX) {
                patchJump(exitJump);
                emitOp(OpCode::OP_POP, 0); // pop condition
            }

            endScope(0);
            break;
        }
    }
}

void Compiler::compileExpr(const Expr* expr) {
    switch (expr->getType()) {
        case ASTNodeType::LITERAL_EXPR: {
            auto lit = static_cast<const LiteralExpr*>(expr);
            if (lit->value.isNil()) {
                emitOp(OpCode::OP_NIL, 0);
            } else if (lit->value.isBool()) {
                emitOp(lit->value.asBool() ? OpCode::OP_TRUE : OpCode::OP_FALSE, 0);
            } else {
                size_t constIdx = currentContext_->chunk.addConstant(lit->value);
                emitOp(OpCode::OP_CONSTANT, 0);
                emitByte(static_cast<uint8_t>(constIdx), 0);
            }
            break;
        }

        case ASTNodeType::VARIABLE_EXPR: {
            auto var = static_cast<const VariableExpr*>(expr);
            int localIdx = resolveLocal(var->name);
            if (localIdx != -1) {
                emitOp(OpCode::OP_GET_LOCAL, var->name.line);
                emitByte(static_cast<uint8_t>(localIdx), var->name.line);
            } else {
                size_t globalIdx = currentContext_->chunk.addConstant(Value(var->name.lexeme));
                emitOp(OpCode::OP_GET_GLOBAL, var->name.line);
                emitByte(static_cast<uint8_t>(globalIdx), var->name.line);
            }
            break;
        }

        case ASTNodeType::ASSIGN_EXPR: {
            auto assign = static_cast<const AssignExpr*>(expr);
            compileExpr(assign->value.get());
            int localIdx = resolveLocal(assign->name);
            if (localIdx != -1) {
                emitOp(OpCode::OP_SET_LOCAL, assign->name.line);
                emitByte(static_cast<uint8_t>(localIdx), assign->name.line);
            } else {
                size_t globalIdx = currentContext_->chunk.addConstant(Value(assign->name.lexeme));
                emitOp(OpCode::OP_SET_GLOBAL, assign->name.line);
                emitByte(static_cast<uint8_t>(globalIdx), assign->name.line);
            }
            break;
        }

        case ASTNodeType::UNARY_EXPR: {
            auto unary = static_cast<const UnaryExpr*>(expr);
            compileExpr(unary->right.get());
            if (unary->op.type == TokenType::MINUS) {
                emitOp(OpCode::OP_NEGATE, unary->op.line);
            } else if (unary->op.type == TokenType::BANG) {
                emitOp(OpCode::OP_NOT, unary->op.line);
            }
            break;
        }

        case ASTNodeType::BINARY_EXPR: {
            auto bin = static_cast<const BinaryExpr*>(expr);
            compileExpr(bin->left.get());
            compileExpr(bin->right.get());

            switch (bin->op.type) {
                case TokenType::PLUS: emitOp(OpCode::OP_ADD, bin->op.line); break;
                case TokenType::MINUS: emitOp(OpCode::OP_SUBTRACT, bin->op.line); break;
                case TokenType::STAR: emitOp(OpCode::OP_MULTIPLY, bin->op.line); break;
                case TokenType::SLASH: emitOp(OpCode::OP_DIVIDE, bin->op.line); break;
                case TokenType::PERCENT: emitOp(OpCode::OP_MODULO, bin->op.line); break;
                case TokenType::EQUAL_EQUAL: emitOp(OpCode::OP_EQUAL, bin->op.line); break;
                case TokenType::BANG_EQUAL:
                    emitOp(OpCode::OP_EQUAL, bin->op.line);
                    emitOp(OpCode::OP_NOT, bin->op.line);
                    break;
                case TokenType::GREATER: emitOp(OpCode::OP_GREATER, bin->op.line); break;
                case TokenType::GREATER_EQUAL:
                    emitOp(OpCode::OP_LESS, bin->op.line);
                    emitOp(OpCode::OP_NOT, bin->op.line);
                    break;
                case TokenType::LESS: emitOp(OpCode::OP_LESS, bin->op.line); break;
                case TokenType::LESS_EQUAL:
                    emitOp(OpCode::OP_GREATER, bin->op.line);
                    emitOp(OpCode::OP_NOT, bin->op.line);
                    break;
                default: break;
            }
            break;
        }

        case ASTNodeType::CALL_EXPR: {
            auto call = static_cast<const CallExpr*>(expr);
            compileExpr(call->callee.get());
            for (const auto& arg : call->arguments) {
                compileExpr(arg.get());
            }
            emitOp(OpCode::OP_CALL, call->paren.line);
            emitByte(static_cast<uint8_t>(call->arguments.size()), call->paren.line);
            break;
        }

        // obj.field  →  map_get(obj, "field")
        case ASTNodeType::GET_FIELD_EXPR: {
            auto getField = static_cast<const GetFieldExpr*>(expr);
            int ln = static_cast<int>(getField->field.line);
            // dispatch to map_get native
            size_t fnIdx = currentContext_->chunk.addConstant(Value(std::string("map_get")));
            emitOp(OpCode::OP_GET_GLOBAL, ln);
            emitByte(static_cast<uint8_t>(fnIdx), ln);
            // obj
            compileExpr(getField->object.get());
            // "field"
            size_t fieldIdx = currentContext_->chunk.addConstant(Value(getField->field.lexeme));
            emitOp(OpCode::OP_CONSTANT, ln);
            emitByte(static_cast<uint8_t>(fieldIdx), ln);
            // call map_get(obj, "field") — 2 arg
            emitOp(OpCode::OP_CALL, ln);
            emitByte(2, ln);
            break;
        }

        // obj.field = value  →  map_set(obj, "field", value)
        case ASTNodeType::SET_FIELD_EXPR: {
            auto setField = static_cast<const SetFieldExpr*>(expr);
            int ln = static_cast<int>(setField->field.line);
            // dispatch to map_set native
            size_t fnIdx = currentContext_->chunk.addConstant(Value(std::string("map_set")));
            emitOp(OpCode::OP_GET_GLOBAL, ln);
            emitByte(static_cast<uint8_t>(fnIdx), ln);
            // obj
            compileExpr(setField->object.get());
            // "field"
            size_t fieldIdx = currentContext_->chunk.addConstant(Value(setField->field.lexeme));
            emitOp(OpCode::OP_CONSTANT, ln);
            emitByte(static_cast<uint8_t>(fieldIdx), ln);
            // value
            compileExpr(setField->value.get());
            // call map_set(obj, "field", val) — 3 arg
            emitOp(OpCode::OP_CALL, ln);
            emitByte(3, ln);
            break;
        }
    }
}

} // namespace srl
