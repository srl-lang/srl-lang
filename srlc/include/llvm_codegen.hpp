#ifndef SRLC_LLVM_CODEGEN_HPP
#define SRLC_LLVM_CODEGEN_HPP

#include "ast.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace srlc {

class LLVMCodegen {
public:
    LLVMCodegen() = default;
    std::string generate(const std::vector<srl::StmtPtr>& statements);

private:
    std::stringstream out_;
    int tempRegCount_ = 0;
    int labelCount_ = 0;

    std::string newRegister();
    std::string newLabel(const std::string& prefix = "label");

    void emitHeader();
    void emitStmt(const srl::Stmt* stmt);
    std::string emitExpr(const srl::Expr* expr);
};

} // namespace srlc

#endif // SRLC_LLVM_CODEGEN_HPP
