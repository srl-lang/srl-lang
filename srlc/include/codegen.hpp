#ifndef SRLC_CODEGEN_HPP
#define SRLC_CODEGEN_HPP

#include "ast.hpp"
#include <string>
#include <vector>
#include <sstream>

namespace srlc {

class Codegen {
public:
    Codegen() = default;
    std::string generate(const std::vector<srl::StmtPtr>& statements);

private:
    std::stringstream out_;
    int indentLevel_ = 0;

    void indent();
    void emitStmt(const srl::Stmt* stmt);
    void emitExpr(const srl::Expr* expr);
};

} // namespace srlc

#endif // SRLC_CODEGEN_HPP
