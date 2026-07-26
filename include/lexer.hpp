#ifndef SRL_LEXER_HPP
#define SRL_LEXER_HPP

#include "token.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace srl {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> scanTokens();

private:
    std::string source_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    static const std::unordered_map<std::string, TokenType> keywords_;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& lexeme);

    void scanToken();
    void string();
    void number();
    void identifier();
    void skipComment();
};

} // namespace srl

#endif // SRL_LEXER_HPP
