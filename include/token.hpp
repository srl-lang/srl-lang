#ifndef SRL_TOKEN_HPP
#define SRL_TOKEN_HPP

#include <string>
#include <string_view>

namespace srl {

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    COMMA, SEMICOLON, COLON, DOT,
    PLUS, MINUS, STAR, SLASH, PERCENT,

    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    ARROW, // ->
    FAT_ARROW, // =>
    AND, OR, // && ||

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Bitwise operators & keywords
    AMPERSAND, PIPE, CARET, TILDE, BIT_LSHIFT, BIT_RSHIFT,

    // Keywords
    KEYWORD_FN,
    KEYWORD_VAR,
    KEYWORD_CONST,
    KEYWORD_ENUM,
    KEYWORD_CLASS,
    KEYWORD_OPERATOR,
    KEYWORD_PUBLIC,
    KEYWORD_PRIVATE,
    KEYWORD_PROTECTED,
    KEYWORD_THIS,
    KEYWORD_MATCH,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_RETURN,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_NIL,
    KEYWORD_STRUCT,
    KEYWORD_UNION,
    KEYWORD_FOR,
    KEYWORD_IN,
    KEYWORD_CASE,
    KEYWORD_DEFAULT,
    KEYWORD_ASYNC,
    KEYWORD_AWAIT,
    KEYWORD_TRY,
    KEYWORD_CATCH,
    KEYWORD_THROW,
    KEYWORD_DEFER,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,

    // Special
    TOKEN_EOF,
    TOKEN_ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;

    Token(TokenType type, std::string lexeme, size_t line, size_t column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}
};

inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN: return "(";
        case TokenType::RIGHT_PAREN: return ")";
        case TokenType::LEFT_BRACE: return "{";
        case TokenType::RIGHT_BRACE: return "}";
        case TokenType::COMMA: return ",";
        case TokenType::SEMICOLON: return ";";
        case TokenType::COLON: return ":";
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::STAR: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::PERCENT: return "%";
        case TokenType::AMPERSAND: return "&";
        case TokenType::PIPE: return "|";
        case TokenType::CARET: return "^";
        case TokenType::TILDE: return "~";
        case TokenType::BIT_LSHIFT: return "<<";
        case TokenType::BIT_RSHIFT: return ">>";
        case TokenType::EQUAL: return "=";
        case TokenType::EQUAL_EQUAL: return "==";
        case TokenType::FAT_ARROW: return "=>";
        case TokenType::BANG_EQUAL: return "!=";
        case TokenType::LESS: return "<";
        case TokenType::LESS_EQUAL: return "<=";
        case TokenType::GREATER: return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::STRING: return "STRING";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::KEYWORD_FN: return "fn";
        case TokenType::KEYWORD_VAR: return "var";
        case TokenType::KEYWORD_CONST: return "const";
        case TokenType::KEYWORD_ENUM: return "enum";
        case TokenType::KEYWORD_CLASS: return "class";
        case TokenType::KEYWORD_THIS: return "this";
        case TokenType::KEYWORD_MATCH: return "match";
        case TokenType::KEYWORD_IF: return "if";
        case TokenType::KEYWORD_ELSE: return "else";
        case TokenType::KEYWORD_WHILE: return "while";
        case TokenType::KEYWORD_RETURN: return "return";
        case TokenType::KEYWORD_TRUE: return "true";
        case TokenType::KEYWORD_FALSE: return "false";
        case TokenType::KEYWORD_NIL: return "nil";
        case TokenType::KEYWORD_STRUCT: return "struct";
        case TokenType::KEYWORD_FOR: return "for";
        case TokenType::KEYWORD_IN: return "in";
        case TokenType::KEYWORD_CASE: return "case";
        case TokenType::KEYWORD_DEFAULT: return "default";
        case TokenType::KEYWORD_ASYNC: return "async";
        case TokenType::KEYWORD_AWAIT: return "await";
        case TokenType::KEYWORD_TRY: return "try";
        case TokenType::KEYWORD_CATCH: return "catch";
        case TokenType::KEYWORD_THROW: return "throw";
        case TokenType::KEYWORD_DEFER: return "defer";
        case TokenType::KEYWORD_BREAK: return "break";
        case TokenType::KEYWORD_CONTINUE: return "continue";
        case TokenType::TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}


} // namespace srl

#endif // SRL_TOKEN_HPP
