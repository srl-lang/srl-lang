#include "lexer.hpp"
#include <cctype>

namespace srl {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"fn", TokenType::KEYWORD_FN},
    {"var", TokenType::KEYWORD_VAR},
    {"const", TokenType::KEYWORD_CONST},
    {"enum", TokenType::KEYWORD_ENUM},
    {"class", TokenType::KEYWORD_CLASS},
    {"operator", TokenType::KEYWORD_OPERATOR},
    {"public", TokenType::KEYWORD_PUBLIC},
    {"private", TokenType::KEYWORD_PRIVATE},
    {"protected", TokenType::KEYWORD_PROTECTED},
    {"this", TokenType::KEYWORD_THIS},
    {"match", TokenType::KEYWORD_MATCH},
    {"if", TokenType::KEYWORD_IF},
    {"else", TokenType::KEYWORD_ELSE},
    {"while", TokenType::KEYWORD_WHILE},
    {"for", TokenType::KEYWORD_FOR},
    {"in", TokenType::KEYWORD_IN},
    {"case", TokenType::KEYWORD_CASE},
    {"default", TokenType::KEYWORD_DEFAULT},
    {"return", TokenType::KEYWORD_RETURN},
    {"true", TokenType::KEYWORD_TRUE},
    {"false", TokenType::KEYWORD_FALSE},
    {"nil", TokenType::KEYWORD_NIL},
    {"struct", TokenType::KEYWORD_STRUCT},
    {"union", TokenType::KEYWORD_UNION},
    {"async", TokenType::KEYWORD_ASYNC},
    {"await", TokenType::KEYWORD_AWAIT},
    {"try", TokenType::KEYWORD_TRY},
    {"catch", TokenType::KEYWORD_CATCH},
    {"throw", TokenType::KEYWORD_THROW},
    {"defer", TokenType::KEYWORD_DEFER},
    {"break", TokenType::KEYWORD_BREAK},
    {"continue", TokenType::KEYWORD_CONTINUE}
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
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
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
            if (match('=')) {
                addToken(TokenType::EQUAL_EQUAL);
            } else if (match('>')) {
                addToken(TokenType::FAT_ARROW);
            } else {
                addToken(TokenType::EQUAL);
            }
            break;
        case '^': addToken(TokenType::CARET); break;
        case '~': addToken(TokenType::TILDE); break;

        case '<':
            if (match('<')) {
                addToken(TokenType::BIT_LSHIFT);
            } else if (match('=')) {
                addToken(TokenType::LESS_EQUAL);
            } else {
                addToken(TokenType::LESS);
            }
            break;
        case '>':
            if (match('>')) {
                addToken(TokenType::BIT_RSHIFT);
            } else if (match('=')) {
                addToken(TokenType::GREATER_EQUAL);
            } else {
                addToken(TokenType::GREATER);
            }
            break;

        case '&':
            if (match('&')) addToken(TokenType::AND);
            else addToken(TokenType::AMPERSAND);
            break;

        case '|':
            if (match('|')) addToken(TokenType::OR);
            else addToken(TokenType::PIPE);
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
        if (peek() == '$' && peekNext() == '{') {
            addToken(TokenType::STRING, value);
            addToken(TokenType::PLUS);
            addToken(TokenType::IDENTIFIER, "to_string");
            addToken(TokenType::LEFT_PAREN);

            advance(); // consume '$'
            advance(); // consume '{'

            while (peek() != '}' && !isAtEnd()) {
                start_ = current_;
                scanToken();
            }

            if (peek() == '}') {
                advance(); // consume '}'
            }
            addToken(TokenType::RIGHT_PAREN);
            addToken(TokenType::PLUS);
            value = "";
            continue;
        }

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
    if (source_[start_] == '0' && (peek() == 'x' || peek() == 'X')) {
        advance(); // consume 'x' / 'X'
        while (std::isxdigit(peek())) advance();
        std::string hexStr = source_.substr(start_ + 2, current_ - (start_ + 2));
        if (!hexStr.empty()) {
            try {
                unsigned long long val = std::stoull(hexStr, nullptr, 16);
                addToken(TokenType::NUMBER, std::to_string(static_cast<double>(val)));
                return;
            } catch (...) {}
        }
    }

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
