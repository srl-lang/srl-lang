#include "parser.hpp"
#include <iostream>
#include <stdexcept>

namespace srl {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration());
        } catch (const std::runtime_error& e) {
            std::cerr << "[Parse Error] " << e.what() << std::endl;
            // Synchronize parser
            advance();
            while (!isAtEnd()) {
                if (previous().type == TokenType::SEMICOLON) break;
                switch (peek().type) {
                    case TokenType::KEYWORD_FN:
                    case TokenType::KEYWORD_VAR:
                    case TokenType::KEYWORD_IF:
                    case TokenType::KEYWORD_WHILE:
                    case TokenType::KEYWORD_RETURN:
                        break;
                    default:
                        advance();
                        break;
                }
            }
        }
    }
    return statements;
}

StmtPtr Parser::declaration() {
    if (match({TokenType::KEYWORD_FN})) return functionDeclaration();
    if (match({TokenType::KEYWORD_VAR})) return varDeclaration(false);
    if (match({TokenType::KEYWORD_CONST})) return varDeclaration(true);
    if (match({TokenType::KEYWORD_STRUCT})) return structDeclaration();
    return statement();
}

StmtPtr Parser::structDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect struct name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before struct body.");

    std::vector<Token> fields;
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            fields.push_back(consume(TokenType::IDENTIFIER, "Expect field name in struct."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after struct body.");

    return std::make_unique<StructStmt>(std::move(name), std::move(fields));
}

StmtPtr Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // Optional return type annotation (e.g. -> int)
    if (match({TokenType::ARROW})) {
        consume(TokenType::IDENTIFIER, "Expect return type after '->'.");
    }

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    
    std::vector<StmtPtr> body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        body.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");

    return std::make_unique<FunctionStmt>(std::move(name), std::move(parameters), std::move(body));
}

StmtPtr Parser::varDeclaration(bool isConst) {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    
    // Optional type annotation (e.g. : int)
    if (match({TokenType::COLON})) {
        consume(TokenType::IDENTIFIER, "Expect type name after ':'.");
    }

    ExprPtr initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    } else {
        initializer = std::make_unique<LiteralExpr>(Value()); // default nil
    }

    match({TokenType::SEMICOLON}); // optional trailing semicolon
    return std::make_unique<VarStmt>(std::move(name), std::move(initializer), isConst);
}

StmtPtr Parser::statement() {
    if (match({TokenType::KEYWORD_IF})) return ifStatement();
    if (match({TokenType::KEYWORD_WHILE})) return whileStatement();
    if (match({TokenType::KEYWORD_FOR})) return forStatement();
    if (match({TokenType::KEYWORD_RETURN})) return returnStatement();
    if (match({TokenType::LEFT_BRACE})) return blockStatement();
    return expressionStatement();
}

StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

StmtPtr Parser::ifStatement() {
    ExprPtr condition = expression();
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    if (match({TokenType::KEYWORD_ELSE})) {
        elseBranch = statement();
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::whileStatement() {
    ExprPtr condition = expression();
    StmtPtr body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::forStatement() {
    // for ( [init] ; [cond] ; [incr] ) body
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer
    StmtPtr initializer = nullptr;
    if (match({TokenType::SEMICOLON})) {
        // no initializer
    } else if (match({TokenType::KEYWORD_VAR})) {
        initializer = varDeclaration();
        match({TokenType::SEMICOLON}); // consume optional ;
    } else {
        initializer = expressionStatement();
    }

    // Condition
    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after for condition.");

    // Increment
    ExprPtr increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for increment.");

    StmtPtr body = statement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

StmtPtr Parser::returnStatement() {
    Token keyword = previous();
    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON) && !check(TokenType::RIGHT_BRACE) && !check(TokenType::TOKEN_EOF)) {
        value = expression();
    }
    match({TokenType::SEMICOLON});
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    match({TokenType::SEMICOLON});
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        ExprPtr value = assignment();

        if (expr->getType() == ASTNodeType::VARIABLE_EXPR) {
            Token name = static_cast<VariableExpr*>(expr.get())->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(value));
        }

        // obj.field = value  →  SetFieldExpr
        if (expr->getType() == ASTNodeType::GET_FIELD_EXPR) {
            auto* getExpr = static_cast<GetFieldExpr*>(expr.get());
            Token field = getExpr->field;
            ExprPtr object = std::move(getExpr->object);
            return std::make_unique<SetFieldExpr>(std::move(object), std::move(field), std::move(value));
        }

        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();
    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }
    return call();
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(std::move(expr));
        } else if (match({TokenType::DOT})) {
            // obj.field  →  GetFieldExpr
            Token field = consume(TokenType::IDENTIFIER, "Expect field name after '.'.");
            expr = std::make_unique<GetFieldExpr>(std::move(expr), std::move(field));
        } else {
            break;
        }
    }

    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }

    Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return std::make_unique<CallExpr>(std::move(callee), std::move(paren), std::move(arguments));
}

ExprPtr Parser::primary() {
    if (match({TokenType::KEYWORD_FALSE})) return std::make_unique<LiteralExpr>(Value(false));
    if (match({TokenType::KEYWORD_TRUE})) return std::make_unique<LiteralExpr>(Value(true));
    if (match({TokenType::KEYWORD_NIL})) return std::make_unique<LiteralExpr>(Value());
    if (match({TokenType::KEYWORD_MATCH})) return matchExpression();

    if (match({TokenType::NUMBER})) {
        double val = std::stod(previous().lexeme);
        return std::make_unique<LiteralExpr>(Value(val));
    }

    if (match({TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(Value(previous().lexeme));
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    throw std::runtime_error("Expect expression at token: " + previous().lexeme);
}

ExprPtr Parser::matchExpression() {
    bool hasParen = match({TokenType::LEFT_PAREN});
    ExprPtr target = expression();
    if (hasParen) {
        consume(TokenType::RIGHT_PAREN, "Expect ')' after match target.");
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' before match cases.");

    std::vector<MatchCase> cases;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        ExprPtr pattern = nullptr;
        if (check(TokenType::IDENTIFIER) && peek().lexeme == "_") {
            advance(); // consume wildcard '_'
        } else {
            pattern = expression();
        }
        consume(TokenType::FAT_ARROW, "Expect '=>' after match pattern.");
        ExprPtr result = expression();
        cases.emplace_back(std::move(pattern), std::move(result));
        if (check(TokenType::COMMA)) advance();
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after match cases.");
    return std::make_unique<MatchExpr>(std::move(target), std::move(cases));
}

// Helpers
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

Token Parser::peek() const {
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error("Line " + std::to_string(peek().line) + ": " + message);
}

} // namespace srl
