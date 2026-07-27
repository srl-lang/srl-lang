#ifndef SRL_PARSER_HPP
#define SRL_PARSER_HPP

#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>

namespace srl {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<StmtPtr> parse();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;

    // Statement parsers
    StmtPtr declaration();
    StmtPtr varDeclaration(bool isConst = false);
    StmtPtr functionDeclaration();
    StmtPtr structDeclaration();
    StmtPtr statement();
    StmtPtr blockStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr returnStatement();
    StmtPtr expressionStatement();

    // Expression parsers
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr finishCall(ExprPtr callee);
    ExprPtr primary();
    ExprPtr matchExpression();

    // Parser utility helpers
    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
};

} // namespace srl

#endif // SRL_PARSER_HPP
