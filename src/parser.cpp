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
    if (match({TokenType::KEYWORD_ASYNC})) {
        consume(TokenType::KEYWORD_FN, "Expect 'fn' after 'async'.");
        return functionDeclaration(true);
    }
    if (match({TokenType::KEYWORD_FN})) return functionDeclaration(false);
    if (match({TokenType::KEYWORD_VAR})) return varDeclaration(false);
    if (match({TokenType::KEYWORD_CONST})) return varDeclaration(true);
    if (match({TokenType::KEYWORD_STRUCT})) return structDeclaration();
    if (match({TokenType::KEYWORD_UNION})) return unionDeclaration();
    if (match({TokenType::KEYWORD_ENUM})) return enumDeclaration();
    if (match({TokenType::KEYWORD_CLASS})) return classDeclaration();
    return statement();
}

StmtPtr Parser::enumDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect enum name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before enum body.");

    std::vector<EnumItem> items;
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            Token itemToken = consume(TokenType::IDENTIFIER, "Expect enum item name.");
            ExprPtr valExpr = nullptr;
            if (match({TokenType::EQUAL})) {
                valExpr = expression();
            }
            items.emplace_back(itemToken, std::move(valExpr));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after enum body.");
    return std::make_unique<EnumStmt>(std::move(name), std::move(items));
}

StmtPtr Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect class name.");
    if (match({TokenType::LESS})) {
        do {
            consume(TokenType::IDENTIFIER, "Expect generic type parameter name.");
        } while (match({TokenType::COMMA}));
        consume(TokenType::GREATER, "Expect '>' after generic type parameters.");
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    std::vector<Token> fields;
    std::vector<ClassField> classFields;
    std::vector<ClassMethod> methods;

    AccessModifier currentAccess = AccessModifier::PUBLIC;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::KEYWORD_PUBLIC})) {
            currentAccess = AccessModifier::PUBLIC;
            match({TokenType::COLON});
        } else if (match({TokenType::KEYWORD_PRIVATE})) {
            currentAccess = AccessModifier::PRIVATE;
            match({TokenType::COLON});
        } else if (match({TokenType::KEYWORD_PROTECTED})) {
            currentAccess = AccessModifier::PROTECTED;
            match({TokenType::COLON});
        } else if (match({TokenType::KEYWORD_VAR})) {
            do {
                Token fTok = consume(TokenType::IDENTIFIER, "Expect field name in class.");
                fields.push_back(fTok);
                classFields.emplace_back(fTok, currentAccess);
            } while (match({TokenType::COMMA}));
            match({TokenType::SEMICOLON});
        } else if (match({TokenType::KEYWORD_FN})) {
            Token methodName(TokenType::IDENTIFIER, "", 0, 0);
            if (match({TokenType::KEYWORD_OPERATOR})) {
                std::string opName = "";
                int line = previous().line;
                int col = previous().column;
                if (match({TokenType::PLUS})) opName = "__add";
                else if (match({TokenType::MINUS})) opName = "__sub";
                else if (match({TokenType::STAR})) opName = "__mul";
                else if (match({TokenType::SLASH})) opName = "__div";
                else if (match({TokenType::PERCENT})) opName = "__mod";
                else if (match({TokenType::EQUAL_EQUAL})) opName = "__eq";
                else if (match({TokenType::BANG_EQUAL})) opName = "__ne";
                else if (check(TokenType::LEFT_PAREN)) {
                    advance();
                    consume(TokenType::RIGHT_PAREN, "Expect ')' after '(' for operator().");
                    opName = "__call";
                } else if (check(TokenType::LEFT_BRACKET)) {
                    advance();
                    consume(TokenType::RIGHT_BRACKET, "Expect ']' after '[' for operator[].");
                    opName = "__index";
                } else {
                    throw std::runtime_error("Expect operator symbol (+, -, *, /, %, [], (), ==, !=) after 'operator'.");
                }
                methodName = Token(TokenType::IDENTIFIER, opName, line, col);
            } else {
                methodName = consume(TokenType::IDENTIFIER, "Expect method name.");
            }
            if (match({TokenType::LESS})) {
                do {
                    consume(TokenType::IDENTIFIER, "Expect generic type parameter name.");
                } while (match({TokenType::COMMA}));
                consume(TokenType::GREATER, "Expect '>' after generic type parameters.");
            }
            consume(TokenType::LEFT_PAREN, "Expect '(' after method name.");
            std::vector<Param> params;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    Token pName = consume(TokenType::IDENTIFIER, "Expect parameter name.");
                    std::string pType = "";
                    if (match({TokenType::COLON})) {
                        Token typeTok = consume(TokenType::IDENTIFIER, "Expect parameter type after ':'.");
                        pType = typeTok.lexeme;
                    }
                    params.emplace_back(std::move(pName), std::move(pType));
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

            std::string returnType = "";
            if (match({TokenType::ARROW}) || match({TokenType::COLON})) {
                Token typeTok = consume(TokenType::IDENTIFIER, "Expect return type after '->' or ':'.");
                returnType = typeTok.lexeme;
            }

            consume(TokenType::LEFT_BRACE, "Expect '{' before method body.");

            std::vector<StmtPtr> body;
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
                body.push_back(declaration());
            }
            consume(TokenType::RIGHT_BRACE, "Expect '}' after method body.");
            methods.emplace_back(methodName, std::move(params), std::move(body), false, std::move(returnType), currentAccess);
        } else {
            advance();
        }
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return std::make_unique<ClassStmt>(std::move(name), std::move(fields), std::move(methods), std::move(classFields));
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

StmtPtr Parser::unionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect union name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before union body.");

    std::vector<Token> fields;
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            fields.push_back(consume(TokenType::IDENTIFIER, "Expect field name in union."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after union body.");

    return std::make_unique<UnionStmt>(std::move(name), std::move(fields));
}

StmtPtr Parser::functionDeclaration(bool isAsync) {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    if (match({TokenType::LESS})) {
        do {
            consume(TokenType::IDENTIFIER, "Expect generic type parameter name.");
        } while (match({TokenType::COMMA}));
        consume(TokenType::GREATER, "Expect '>' after generic type parameters.");
    }
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    
    std::vector<Param> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token pName = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            std::string pType = "";
            if (match({TokenType::COLON})) {
                Token typeTok = consume(TokenType::IDENTIFIER, "Expect parameter type after ':'.");
                pType = typeTok.lexeme;
            }
            parameters.emplace_back(std::move(pName), std::move(pType));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // Optional return type annotation (e.g. -> int or : int)
    std::string returnType = "";
    if (match({TokenType::ARROW}) || match({TokenType::COLON})) {
        Token typeTok = consume(TokenType::IDENTIFIER, "Expect return type after '->' or ':'.");
        returnType = typeTok.lexeme;
    }

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    
    std::vector<StmtPtr> body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        body.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");

    return std::make_unique<FunctionStmt>(std::move(name), std::move(parameters), std::move(body), isAsync, std::move(returnType));
}

StmtPtr Parser::varDeclaration(bool isConst) {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    
    // Optional type annotation (e.g. : int)
    std::string typeAnn = "";
    if (match({TokenType::COLON})) {
        Token typeTok = consume(TokenType::IDENTIFIER, "Expect type name after ':'.");
        typeAnn = typeTok.lexeme;
    }

    ExprPtr initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    } else {
        initializer = std::make_unique<LiteralExpr>(Value()); // default nil
    }

    match({TokenType::SEMICOLON}); // optional trailing semicolon
    return std::make_unique<VarStmt>(std::move(name), std::move(initializer), isConst, std::move(typeAnn));
}

StmtPtr Parser::statement() {
    if (match({TokenType::KEYWORD_IF})) return ifStatement();
    if (match({TokenType::KEYWORD_WHILE})) return whileStatement();
    if (match({TokenType::KEYWORD_FOR})) return forStatement();
    if (match({TokenType::KEYWORD_RETURN})) return returnStatement();
    if (match({TokenType::KEYWORD_TRY})) return tryCatchStatement();
    if (match({TokenType::KEYWORD_THROW})) return throwStatement();
    if (match({TokenType::KEYWORD_BREAK})) {
        Token kw = previous();
        match({TokenType::SEMICOLON});
        return std::make_unique<BreakStmt>(std::move(kw));
    }
    if (match({TokenType::KEYWORD_CONTINUE})) {
        Token kw = previous();
        match({TokenType::SEMICOLON});
        return std::make_unique<ContinueStmt>(std::move(kw));
    }
    if (match({TokenType::LEFT_BRACE})) return blockStatement();
    return expressionStatement();
}

StmtPtr Parser::tryCatchStatement() {
    StmtPtr tryBranch = statement();
    consume(TokenType::KEYWORD_CATCH, "Expect 'catch' after try block.");
    bool hasParen = match({TokenType::LEFT_PAREN});
    Token catchVar = consume(TokenType::IDENTIFIER, "Expect catch variable name.");
    if (hasParen) {
        consume(TokenType::RIGHT_PAREN, "Expect ')' after catch variable.");
    }
    StmtPtr catchBranch = statement();
    return std::make_unique<TryCatchStmt>(std::move(tryBranch), std::move(catchVar), std::move(catchBranch));
}

StmtPtr Parser::throwStatement() {
    Token keyword = previous();
    ExprPtr expr = expression();
    match({TokenType::SEMICOLON});
    return std::make_unique<ThrowStmt>(std::move(keyword), std::move(expr));
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
    // OR for (var item in iterable) body
    // OR for (var key, val in map) body
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    if (match({TokenType::KEYWORD_VAR})) {
        Token firstVar = consume(TokenType::IDENTIFIER, "Expect variable name after 'var'.");
        if (match({TokenType::COMMA})) {
            Token secondVar = consume(TokenType::IDENTIFIER, "Expect second variable name after ','.");
            consume(TokenType::KEYWORD_IN, "Expect 'in' after variable names in for-in loop.");
            ExprPtr collection = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after for-in expression.");
            StmtPtr body = statement();

            static size_t loopId = 0;
            std::string idStr = std::to_string(++loopId);
            Token collTok(TokenType::IDENTIFIER, "_coll_" + idStr, previous().line, previous().column);
            Token keysTok(TokenType::IDENTIFIER, "_keys_" + idStr, previous().line, previous().column);
            Token iTok(TokenType::IDENTIFIER, "_i_" + idStr, previous().line, previous().column);
            Token lenTok(TokenType::IDENTIFIER, "_len_" + idStr, previous().line, previous().column);
            Token parenTok(TokenType::LEFT_PAREN, "(", previous().line, previous().column);

            std::vector<StmtPtr> desugaredStmts;
            desugaredStmts.push_back(std::make_unique<VarStmt>(collTok, std::move(collection)));

            std::vector<ExprPtr> mkArgs;
            mkArgs.push_back(std::make_unique<VariableExpr>(collTok));
            ExprPtr mapKeysCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "map_keys", 0, 0)), parenTok, std::move(mkArgs));
            desugaredStmts.push_back(std::make_unique<VarStmt>(keysTok, std::move(mapKeysCall)));

            std::vector<ExprPtr> alArgs;
            alArgs.push_back(std::make_unique<VariableExpr>(keysTok));
            ExprPtr arrLenCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr_len", 0, 0)), parenTok, std::move(alArgs));
            desugaredStmts.push_back(std::make_unique<VarStmt>(lenTok, std::move(arrLenCall)));

            StmtPtr loopInit = std::make_unique<VarStmt>(iTok, std::make_unique<LiteralExpr>(Value(0.0)));
            ExprPtr loopCond = std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(iTok), Token(TokenType::LESS, "<", 0, 0), std::make_unique<VariableExpr>(lenTok));
            ExprPtr loopIncr = std::make_unique<AssignExpr>(iTok, std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(iTok), Token(TokenType::PLUS, "+", 0, 0), std::make_unique<LiteralExpr>(Value(1.0))));

            std::vector<StmtPtr> innerBody;
            std::vector<ExprPtr> agArgs;
            agArgs.push_back(std::make_unique<VariableExpr>(keysTok));
            agArgs.push_back(std::make_unique<VariableExpr>(iTok));
            ExprPtr arrGetCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr_get", 0, 0)), parenTok, std::move(agArgs));
            innerBody.push_back(std::make_unique<VarStmt>(firstVar, std::move(arrGetCall)));

            std::vector<ExprPtr> mgArgs;
            mgArgs.push_back(std::make_unique<VariableExpr>(collTok));
            mgArgs.push_back(std::make_unique<VariableExpr>(firstVar));
            ExprPtr mapGetCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "map_get", 0, 0)), parenTok, std::move(mgArgs));
            innerBody.push_back(std::make_unique<VarStmt>(secondVar, std::move(mapGetCall)));

            innerBody.push_back(std::move(body));

            StmtPtr innerBlock = std::make_unique<BlockStmt>(std::move(innerBody));
            desugaredStmts.push_back(std::make_unique<ForStmt>(std::move(loopInit), std::move(loopCond), std::move(loopIncr), std::move(innerBlock)));

            return std::make_unique<BlockStmt>(std::move(desugaredStmts));
        } else if (match({TokenType::KEYWORD_IN})) {
            ExprPtr collection = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after for-in expression.");
            StmtPtr body = statement();

            static size_t loopId = 0;
            std::string idStr = std::to_string(++loopId);
            Token collTok(TokenType::IDENTIFIER, "_coll_" + idStr, previous().line, previous().column);
            Token iTok(TokenType::IDENTIFIER, "_i_" + idStr, previous().line, previous().column);
            Token lenTok(TokenType::IDENTIFIER, "_len_" + idStr, previous().line, previous().column);
            Token parenTok(TokenType::LEFT_PAREN, "(", previous().line, previous().column);

            std::vector<StmtPtr> desugaredStmts;
            desugaredStmts.push_back(std::make_unique<VarStmt>(collTok, std::move(collection)));

            std::vector<ExprPtr> alArgs;
            alArgs.push_back(std::make_unique<VariableExpr>(collTok));
            ExprPtr arrLenCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr_len", 0, 0)), parenTok, std::move(alArgs));
            desugaredStmts.push_back(std::make_unique<VarStmt>(lenTok, std::move(arrLenCall)));

            StmtPtr loopInit = std::make_unique<VarStmt>(iTok, std::make_unique<LiteralExpr>(Value(0.0)));
            ExprPtr loopCond = std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(iTok), Token(TokenType::LESS, "<", 0, 0), std::make_unique<VariableExpr>(lenTok));
            ExprPtr loopIncr = std::make_unique<AssignExpr>(iTok, std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(iTok), Token(TokenType::PLUS, "+", 0, 0), std::make_unique<LiteralExpr>(Value(1.0))));

            std::vector<StmtPtr> innerBody;
            std::vector<ExprPtr> agArgs;
            agArgs.push_back(std::make_unique<VariableExpr>(collTok));
            agArgs.push_back(std::make_unique<VariableExpr>(iTok));
            ExprPtr arrGetCall = std::make_unique<CallExpr>(std::make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr_get", 0, 0)), parenTok, std::move(agArgs));
            innerBody.push_back(std::make_unique<VarStmt>(firstVar, std::move(arrGetCall)));

            innerBody.push_back(std::move(body));

            StmtPtr innerBlock = std::make_unique<BlockStmt>(std::move(innerBody));
            desugaredStmts.push_back(std::make_unique<ForStmt>(std::move(loopInit), std::move(loopCond), std::move(loopIncr), std::move(innerBlock)));

            return std::make_unique<BlockStmt>(std::move(desugaredStmts));
        } else {
            ExprPtr initializer = nullptr;
            if (match({TokenType::EQUAL})) {
                initializer = expression();
            }
            match({TokenType::SEMICOLON});
            StmtPtr initStmt = std::make_unique<VarStmt>(std::move(firstVar), std::move(initializer));

            ExprPtr condition = nullptr;
            if (!check(TokenType::SEMICOLON)) {
                condition = expression();
            }
            consume(TokenType::SEMICOLON, "Expect ';' after for condition.");

            ExprPtr increment = nullptr;
            if (!check(TokenType::RIGHT_PAREN)) {
                increment = expression();
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after for increment.");

            StmtPtr body = statement();
            return std::make_unique<ForStmt>(std::move(initStmt), std::move(condition), std::move(increment), std::move(body));
        }
    }

    StmtPtr initializer = nullptr;
    if (match({TokenType::SEMICOLON})) {
        // no initializer
    } else {
        initializer = expressionStatement();
    }

    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after for condition.");

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
    ExprPtr expr = bitwiseOr();
    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = bitwiseOr();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::bitwiseOr() {
    ExprPtr expr = bitwiseXor();
    while (match({TokenType::PIPE})) {
        Token op = previous();
        ExprPtr right = bitwiseXor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::bitwiseXor() {
    ExprPtr expr = bitwiseAnd();
    while (match({TokenType::CARET})) {
        Token op = previous();
        ExprPtr right = bitwiseAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::bitwiseAnd() {
    ExprPtr expr = equality();
    while (match({TokenType::AMPERSAND})) {
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
    ExprPtr expr = shift();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = shift();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

ExprPtr Parser::shift() {
    ExprPtr expr = term();
    while (match({TokenType::BIT_LSHIFT, TokenType::BIT_RSHIFT})) {
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
    if (match({TokenType::KEYWORD_AWAIT})) {
        Token keyword = previous();
        ExprPtr right = unary();
        return std::make_unique<AwaitExpr>(std::move(keyword), std::move(right));
    }
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::TILDE})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }
    return call();
}


ExprPtr Parser::call() {
    ExprPtr expr = primary();

    while (true) {
        if (check(TokenType::LESS) && (expr->getType() == ASTNodeType::VARIABLE_EXPR || expr->getType() == ASTNodeType::GET_FIELD_EXPR)) {
            size_t saved = current_;
            advance();
            bool isGenericCall = false;
            if (check(TokenType::IDENTIFIER)) {
                advance();
                if (check(TokenType::COMMA) || check(TokenType::GREATER)) {
                    isGenericCall = true;
                }
            }
            current_ = saved;

            if (isGenericCall) {
                advance();
                do {
                    consume(TokenType::IDENTIFIER, "Expect generic type argument.");
                } while (match({TokenType::COMMA}));
                consume(TokenType::GREATER, "Expect '>' after generic type arguments.");
            } else {
                break;
            }
        } else if (match({TokenType::LEFT_PAREN})) {
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

ExprPtr Parser::matchExpression() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'match'.");
    ExprPtr target = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after match target expression.");

    consume(TokenType::LEFT_BRACE, "Expect '{' before match cases.");

    std::vector<MatchCase> cases;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::KEYWORD_CASE})) {
            ExprPtr pattern = expression();
            if (!match({TokenType::FAT_ARROW}) && !match({TokenType::ARROW})) {
                consume(TokenType::FAT_ARROW, "Expect '=>' after match case pattern.");
            }
            ExprPtr result = expression();
            match({TokenType::COMMA, TokenType::SEMICOLON});
            cases.emplace_back(std::move(pattern), std::move(result));
        } else if (match({TokenType::KEYWORD_DEFAULT})) {
            if (!match({TokenType::FAT_ARROW}) && !match({TokenType::ARROW})) {
                consume(TokenType::FAT_ARROW, "Expect '=>' after 'default'.");
            }
            ExprPtr result = expression();
            match({TokenType::COMMA, TokenType::SEMICOLON});
            cases.emplace_back(nullptr, std::move(result));
        } else if (check(TokenType::IDENTIFIER) && peek().lexeme == "_") {
            advance();
            if (!match({TokenType::FAT_ARROW}) && !match({TokenType::ARROW})) {
                consume(TokenType::FAT_ARROW, "Expect '=>' after '_'.");
            }
            ExprPtr result = expression();
            match({TokenType::COMMA, TokenType::SEMICOLON});
            cases.emplace_back(nullptr, std::move(result));
        } else {
            ExprPtr pattern = expression();
            if (!match({TokenType::FAT_ARROW}) && !match({TokenType::ARROW})) {
                consume(TokenType::FAT_ARROW, "Expect '=>' after match pattern.");
            }
            ExprPtr result = expression();
            match({TokenType::COMMA, TokenType::SEMICOLON});
            cases.emplace_back(std::move(pattern), std::move(result));
        }
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after match cases.");
    return std::make_unique<MatchExpr>(std::move(target), std::move(cases));
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

    if (match({TokenType::IDENTIFIER, TokenType::KEYWORD_THIS})) {
        return std::make_unique<VariableExpr>(previous());
    }


    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    throw std::runtime_error("Expect expression at token: " + previous().lexeme);
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
