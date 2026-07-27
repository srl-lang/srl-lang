#ifndef SRL_AST_HPP
#define SRL_AST_HPP

#include "token.hpp"
#include "value.hpp"
#include <memory>
#include <vector>

namespace srl {

struct Expr;
struct Stmt;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

enum class ASTNodeType {
    // Expressions
    LITERAL_EXPR,
    VARIABLE_EXPR,
    ASSIGN_EXPR,
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR,
    GET_FIELD_EXPR,
    SET_FIELD_EXPR,
    MATCH_EXPR,

    // Statements
    EXPRESSION_STMT,
    VAR_STMT,
    BLOCK_STMT,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    FUNCTION_STMT,
    STRUCT_STMT
};

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
};

struct Expr : public ASTNode {};
struct Stmt : public ASTNode {};

// Expressions
struct LiteralExpr : public Expr {
    Value value;
    explicit LiteralExpr(Value val) : value(std::move(val)) {}
    ASTNodeType getType() const override { return ASTNodeType::LITERAL_EXPR; }
};

struct VariableExpr : public Expr {
    Token name;
    explicit VariableExpr(Token name) : name(std::move(name)) {}
    ASTNodeType getType() const override { return ASTNodeType::VARIABLE_EXPR; }
};

struct AssignExpr : public Expr {
    Token name;
    ExprPtr value;
    AssignExpr(Token name, ExprPtr val) : name(std::move(name)), value(std::move(val)) {}
    ASTNodeType getType() const override { return ASTNodeType::ASSIGN_EXPR; }
};

struct BinaryExpr : public Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
    ASTNodeType getType() const override { return ASTNodeType::BINARY_EXPR; }
};

struct UnaryExpr : public Expr {
    Token op;
    ExprPtr right;
    UnaryExpr(Token op, ExprPtr r) : op(std::move(op)), right(std::move(r)) {}
    ASTNodeType getType() const override { return ASTNodeType::UNARY_EXPR; }
};

struct CallExpr : public Expr {
    ExprPtr callee;
    Token paren;
    std::vector<ExprPtr> arguments;
    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> args)
        : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(args)) {}
    ASTNodeType getType() const override { return ASTNodeType::CALL_EXPR; }
};

// obj.field -- read field
struct GetFieldExpr : public Expr {
    ExprPtr object;
    Token field;
    GetFieldExpr(ExprPtr obj, Token field) : object(std::move(obj)), field(std::move(field)) {}
    ASTNodeType getType() const override { return ASTNodeType::GET_FIELD_EXPR; }
};

// obj.field = value -- write field
struct SetFieldExpr : public Expr {
    ExprPtr object;
    Token field;
    ExprPtr value;
    SetFieldExpr(ExprPtr obj, Token field, ExprPtr val)
        : object(std::move(obj)), field(std::move(field)), value(std::move(val)) {}
    ASTNodeType getType() const override { return ASTNodeType::SET_FIELD_EXPR; }
};

struct MatchCase {
    ExprPtr pattern; // nullptr if wildcard '_'
    ExprPtr result;
    MatchCase(ExprPtr pat, ExprPtr res) : pattern(std::move(pat)), result(std::move(res)) {}
};

struct MatchExpr : public Expr {
    ExprPtr target;
    std::vector<MatchCase> cases;
    MatchExpr(ExprPtr target, std::vector<MatchCase> cases)
        : target(std::move(target)), cases(std::move(cases)) {}
    ASTNodeType getType() const override { return ASTNodeType::MATCH_EXPR; }
};

// Statements
struct ExpressionStmt : public Stmt {
    ExprPtr expression;
    explicit ExpressionStmt(ExprPtr expr) : expression(std::move(expr)) {}
    ASTNodeType getType() const override { return ASTNodeType::EXPRESSION_STMT; }
};

struct VarStmt : public Stmt {
    Token name;
    ExprPtr initializer;
    bool isConst;
    VarStmt(Token name, ExprPtr init, bool isConst = false)
        : name(std::move(name)), initializer(std::move(init)), isConst(isConst) {}
    ASTNodeType getType() const override { return ASTNodeType::VAR_STMT; }
};

struct BlockStmt : public Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
    ASTNodeType getType() const override { return ASTNodeType::BLOCK_STMT; }
};

struct IfStmt : public Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr cond, StmtPtr thenB, StmtPtr elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    ASTNodeType getType() const override { return ASTNodeType::IF_STMT; }
};

struct WhileStmt : public Stmt {
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr cond, StmtPtr body) : condition(std::move(cond)), body(std::move(body)) {}
    ASTNodeType getType() const override { return ASTNodeType::WHILE_STMT; }
};

struct ReturnStmt : public Stmt {
    Token keyword;
    ExprPtr value;
    ReturnStmt(Token keyword, ExprPtr val) : keyword(std::move(keyword)), value(std::move(val)) {}
    ASTNodeType getType() const override { return ASTNodeType::RETURN_STMT; }
};

struct FunctionStmt : public Stmt {
    Token name;
    std::vector<Token> params;
    std::vector<StmtPtr> body;
    FunctionStmt(Token name, std::vector<Token> params, std::vector<StmtPtr> body)
        : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}
    ASTNodeType getType() const override { return ASTNodeType::FUNCTION_STMT; }
};

struct StructStmt : public Stmt {
    Token name;
    std::vector<Token> fields;
    StructStmt(Token name, std::vector<Token> fields)
        : name(std::move(name)), fields(std::move(fields)) {}
    ASTNodeType getType() const override { return ASTNodeType::STRUCT_STMT; }
};

// for (init; condition; increment) body
struct ForStmt : public Stmt {
    StmtPtr initializer;  // nullable
    ExprPtr condition;    // nullable (infinite loop if absent)
    ExprPtr increment;    // nullable
    StmtPtr body;
    ForStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr body)
        : initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(incr)), body(std::move(body)) {}
    ASTNodeType getType() const override { return ASTNodeType::FOR_STMT; }
};

} // namespace srl

#endif // SRL_AST_HPP
