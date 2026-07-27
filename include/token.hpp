#ifndef SRL_TOKEN_HPP
#define SRL_TOKEN_HPP

#include <string>
#include <string_view>

namespace srl {

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, SEMICOLON, COLON, DOT,
    PLUS, MINUS, STAR, SLASH, PERCENT,

    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    ARROW, // ->
    AND, OR, // && ||

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    KEYWORD_FN,
    KEYWORD_VAR,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_RETURN,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_NIL,
    KEYWORD_STRUCT,
    KEYWORD_FOR,

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
        case TokenType::EQUAL: return "=";
        case TokenType::EQUAL_EQUAL: return "==";
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
        case TokenType::KEYWORD_IF: return "if";
        case TokenType::KEYWORD_ELSE: return "else";
        case TokenType::KEYWORD_WHILE: return "while";
        case TokenType::KEYWORD_RETURN: return "return";
        case TokenType::KEYWORD_TRUE: return "true";
        case TokenType::KEYWORD_FALSE: return "false";
        case TokenType::KEYWORD_NIL: return "nil";
        case TokenType::KEYWORD_STRUCT: return "struct";
        case TokenType::KEYWORD_FOR: return "for";
        case TokenType::TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

} // namespace srl

#endif // SRL_TOKEN_HPP
