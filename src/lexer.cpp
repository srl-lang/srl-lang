#include "lexer.hpp"
#include <cctype>

namespace srl {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"fn", TokenType::KEYWORD_FN},
    {"var", TokenType::KEYWORD_VAR},
    {"if", TokenType::KEYWORD_IF},
    {"else", TokenType::KEYWORD_ELSE},
    {"while", TokenType::KEYWORD_WHILE},
    {"return", TokenType::KEYWORD_RETURN},
    {"true", TokenType::KEYWORD_TRUE},
    {"false", TokenType::KEYWORD_FALSE},
    {"nil", TokenType::KEYWORD_NIL},
    {"struct", TokenType::KEYWORD_STRUCT}
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }

    tokens_.emplace_back(TokenType::TOKEN_EOF, "", line_, column_);
    return tokens_;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

char Lexer::advance() {
    column_++;
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;

    current_++;
    column_++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, text, line_, column_ - text.length());
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    tokens_.emplace_back(type, lexeme, line_, column_ - lexeme.length());
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case ':': addToken(TokenType::COLON); break;
        case '.': addToken(TokenType::DOT); break;
        case '+': addToken(TokenType::PLUS); break;
        case '*': addToken(TokenType::STAR); break;
        case '%': addToken(TokenType::PERCENT); break;
        
        case '-':
            if (match('>')) {
                addToken(TokenType::ARROW);
            } else {
                addToken(TokenType::MINUS);
            }
            break;

        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;

        case '&':
            if (match('&')) addToken(TokenType::AND);
            else addToken(TokenType::TOKEN_ERROR, "Unexpected token '&'");
            break;

        case '|':
            if (match('|')) addToken(TokenType::OR);
            else addToken(TokenType::TOKEN_ERROR, "Unexpected token '|'");
            break;

        case '/':
            if (match('/')) {
                // Single line comment
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                // Multi line comment
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); // '*'
                        advance(); // '/'
                        break;
                    }
                    if (peek() == '\n') {
                        line_++;
                        column_ = 1;
                    }
                    advance();
                }
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;

        case '\n':
            line_++;
            column_ = 1;
            break;

        case '"': string(); break;

        default:
            if (std::isdigit(c)) {
                number();
            } else if (std::isalpha(c) || c == '_') {
                identifier();
            } else {
                addToken(TokenType::TOKEN_ERROR, std::string("Unexpected character: ") + c);
            }
            break;
    }
}

void Lexer::string() {
    std::string value;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }
        if (peek() == '\\') {
            advance(); // escape character
            char escaped = peek();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                default: value += escaped; break;
            }
        } else {
            value += peek();
        }
        advance();
    }

    if (isAtEnd()) {
        addToken(TokenType::TOKEN_ERROR, "Unterminated string");
        return;
    }

    advance(); // Closing '"'
    addToken(TokenType::STRING, value);
}

void Lexer::number() {
    while (std::isdigit(peek())) advance();

    // Look for fractional part
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance(); // consume '.'
        while (std::isdigit(peek())) advance();
    }

    addToken(TokenType::NUMBER, source_.substr(start_, current_ - start_));
}

void Lexer::identifier() {
    while (std::isalnum(peek()) || peek() == '_') advance();

    std::string text = source_.substr(start_, current_ - start_);
    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::IDENTIFIER;
    addToken(type, text);
}

} // namespace srl
