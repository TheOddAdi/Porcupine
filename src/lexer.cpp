#include "lexer.hpp"

#include <cctype>
#include <string>
#include <unordered_map>

// Stores the source code that this lexer will scan.
Lexer::Lexer(const std::string& source)
    : source(source) {
}

// Returns the current character, or '\0' at the end of the source.
char Lexer::current() const {
    if (position >= source.size()) {
        return '\0';
    }

    return source[position];
}

// Returns the next character without consuming it.
char Lexer::peek() const {
    if (position + 1 >= source.size()) {
        return '\0';
    }

    return source[position + 1];
}

// Returns the character two positions ahead without consuming it.
char Lexer::peekNext() const {
    if (position + 2 >= source.size()) {
        return '\0';
    }

    return source[position + 2];
}

// Consumes one character and updates the source location.
char Lexer::advance() {
    if (position >= source.size()) {
        return '\0';
    }

    char c = source[position++];

    if (c == '\n') {
        ++line;
        column = 1;
    } else {
        ++column;
    }

    return c;
}

// Skips spaces, tabs, newlines, and other whitespace.
void Lexer::skipWhitespace() {
    while (position < source.size() &&
           std::isspace(
               static_cast<unsigned char>(current()))) {
        advance();
    }
}

// Skips a // comment until the end of the line.
void Lexer::skipLineComment() {
    while (position < source.size() && current() != '\n') {
        advance();
    }
}

// Skips a /* ... */ comment.
// Reports an error if the closing */ is missing.
void Lexer::skipBlockComment() {
    int startLine = line;
    int startColumn = column;

    // Consume the opening /*
    advance();
    advance();

    while (position < source.size()) {
        if (current() == '*' && peek() == '/') {
            advance();
            advance();
            return;
        }

        advance();
    }

    addError(
        "Unterminated block comment",
        startLine,
        startColumn
    );
}

// Records an error and its source location.
void Lexer::addError(
    const std::string& message,
    int errorLine,
    int errorColumn
) {
    errors.push_back({message, errorLine, errorColumn});
}

// Provides read-only access to errors found by the lexer.
const std::vector<LexerError>& Lexer::getErrors() const {
    return errors;
}

// Scans source code and produces a list of tokens.
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    // Reset state so tokenize() can be called again.
    position = 0;
    line = 1;
    column = 1;
    errors.clear();

    // Maps reserved words to their token types.
    static const std::unordered_map<std::string, TokenType> keywords = {
        // Built-in types
        {"int", TokenType::KeywordInt},
        {"float", TokenType::KeywordFloat},
        {"bool", TokenType::KeywordBool},
        {"char", TokenType::KeywordChar},
        {"void", TokenType::KeywordVoid},
        {"string", TokenType::KeywordString},
        {"unsigned", TokenType::KeywordUnsigned},
        {"var", TokenType::KeywordVar},

        // Declarations
        {"const", TokenType::KeywordConst},
        {"type", TokenType::KeywordType},

        // Control flow
        {"return", TokenType::KeywordReturn},
        {"if", TokenType::KeywordIf},
        {"else", TokenType::KeywordElse},
        {"while", TokenType::KeywordWhile},
        {"for", TokenType::KeywordFor},
        {"do", TokenType::KeywordDo},
        {"break", TokenType::KeywordBreak},
        {"continue", TokenType::KeywordContinue},

        // Comparison statements
        {"compare", TokenType::KeywordCompare},
        {"case", TokenType::KeywordCase},
        {"default", TokenType::KeywordDefault},

        // Special keywords and boolean literals
        {"nullptr", TokenType::KeywordNullptr},
        {"true", TokenType::KeywordTrue},
        {"false", TokenType::KeywordFalse}
    };

    while (position < source.size()) {
        skipWhitespace();

        if (position >= source.size()) {
            break;
        }

        // Comments must be recognized before the / operator.
        if (current() == '/' && peek() == '/') {
            skipLineComment();
            continue;
        }

        if (current() == '/' && peek() == '*') {
            skipBlockComment();
            continue;
        }

        int tokenLine = line;
        int tokenColumn = column;
        char c = current();

        // IDENTIFIERS AND KEYWORDS
        if (std::isalpha(static_cast<unsigned char>(c)) ||
            c == '_') {
            std::string text;

            while (position < source.size() &&
                   (std::isalnum(
                        static_cast<unsigned char>(current())) ||
                    current() == '_')) {
                text += advance();
            }

            TokenType type = TokenType::Identifier;

            auto keyword = keywords.find(text);

            if (keyword != keywords.end()) {
                type = keyword->second;
            }

            tokens.push_back({
                type,
                text,
                tokenLine,
                tokenColumn
            });

            continue;
        }

        // NUMERIC LITERALS
        if (std::isdigit(static_cast<unsigned char>(c))) {
            std::string text;

            // Read the integer portion.
            while (position < source.size() &&
                   std::isdigit(
                       static_cast<unsigned char>(current()))) {
                text += advance();
            }

            // A decimal point followed by a digit makes this a float.
            if (current() == '.' &&
                std::isdigit(
                    static_cast<unsigned char>(peek()))) {
                text += advance();

                while (position < source.size() &&
                       std::isdigit(
                           static_cast<unsigned char>(current()))) {
                    text += advance();
                }

                tokens.push_back({
                    TokenType::FloatLiteral,
                    text,
                    tokenLine,
                    tokenColumn
                });
            } else {
                tokens.push_back({
                    TokenType::IntegerLiteral,
                    text,
                    tokenLine,
                    tokenColumn
                });
            }

            continue;
        }

        // STRING LITERALS
        if (c == '"') {
            std::string text;
            bool terminated = false;

            // Include the opening quote in the token text.
            text += advance();

            while (position < source.size()) {
                if (current() == '"') {
                    text += advance();
                    terminated = true;
                    break;
                }

                // Raw newlines are not allowed in strings.
                if (current() == '\n') {
                    break;
                }

                // Preserve escaped characters.
                if (current() == '\\') {
                    text += advance();

                    if (position >= source.size() ||
                        current() == '\n') {
                        break;
                    }

                    text += advance();
                } else {
                    text += advance();
                }
            }

            if (!terminated) {
                addError(
                    "Unterminated string literal",
                    tokenLine,
                    tokenColumn
                );
            }

            tokens.push_back({
                TokenType::StringLiteral,
                text,
                tokenLine,
                tokenColumn
            });

            continue;
        }

        // MULTI-CHARACTER OPERATORS
        // Check three-character operators first.
        if (c == '<' && peek() == '<' && peekNext() == '=') {
            advance();
            advance();
            advance();

            tokens.push_back({
                TokenType::LeftShiftEquals,
                "<<=",
                tokenLine,
                tokenColumn
            });

            continue;
        }

        if (c == '>' && peek() == '>' && peekNext() == '=') {
            advance();
            advance();
            advance();

            tokens.push_back({
                TokenType::RightShiftEquals,
                ">>=",
                tokenLine,
                tokenColumn
            });

            continue;
        }

        // Check two-character operators.
        TokenType type;
        bool matched = true;
        std::string op;

        if ((c == '+' && peek() == '+') ||
            (c == '-' && peek() == '-') ||
            (c == '+' && peek() == '=') ||
            (c == '-' && peek() == '=') ||
            (c == '*' && peek() == '=') ||
            (c == '/' && peek() == '=') ||
            (c == '%' && peek() == '=') ||
            (c == '=' && peek() == '=') ||
            (c == '!' && peek() == '=') ||
            (c == '<' && peek() == '=') ||
            (c == '>' && peek() == '=') ||
            (c == '&' && peek() == '&') ||
            (c == '|' && peek() == '|') ||
            (c == '<' && peek() == '<') ||
            (c == '>' && peek() == '>') ||
            (c == '&' && peek() == '=') ||
            (c == '|' && peek() == '=') ||
            (c == '^' && peek() == '=') ||
            (c == '-' && peek() == '>')) {

            op += advance();
            op += advance();

            if (op == "++") type = TokenType::PlusPlus;
            else if (op == "--") type = TokenType::MinusMinus;
            else if (op == "+=") type = TokenType::PlusEquals;
            else if (op == "-=") type = TokenType::MinusEquals;
            else if (op == "*=") type = TokenType::StarEquals;
            else if (op == "/=") type = TokenType::SlashEquals;
            else if (op == "%=") type = TokenType::PercentEquals;
            else if (op == "==") type = TokenType::EqualEqual;
            else if (op == "!=") type = TokenType::BangEqual;
            else if (op == "<=") type = TokenType::LessEqual;
            else if (op == ">=") type = TokenType::GreaterEqual;
            else if (op == "&&") type = TokenType::AmpAmp;
            else if (op == "||") type = TokenType::PipePipe;
            else if (op == "<<") type = TokenType::LeftShift;
            else if (op == ">>") type = TokenType::RightShift;
            else if (op == "&=") type = TokenType::AmpEquals;
            else if (op == "|=") type = TokenType::PipeEquals;
            else if (op == "^=") type = TokenType::CaretEquals;
            else if (op == "->") type = TokenType::Arrow;
            else matched = false;

            if (matched) {
                tokens.push_back({
                    type,
                    op,
                    tokenLine,
                    tokenColumn
                });

                continue;
            }
        }

        // SINGLE-CHARACTER OPERATORS AND PUNCTUATION
        switch (c) {
            case '+': type = TokenType::Plus; break;
            case '-': type = TokenType::Minus; break;
            case '*': type = TokenType::Star; break;
            case '/': type = TokenType::Slash; break;
            case '%': type = TokenType::Percent; break;

            case '=': type = TokenType::Equals; break;
            case '!': type = TokenType::Bang; break;
            case '<': type = TokenType::Less; break;
            case '>': type = TokenType::Greater; break;

            case '&': type = TokenType::Ampersand; break;
            case '|': type = TokenType::Pipe; break;
            case '^': type = TokenType::Caret; break;
            case '~': type = TokenType::Tilde; break;

            case '?': type = TokenType::Question; break;
            case ':': type = TokenType::Colon; break;

            case '(': type = TokenType::LeftParen; break;
            case ')': type = TokenType::RightParen; break;
            case '{': type = TokenType::LeftBrace; break;
            case '}': type = TokenType::RightBrace; break;
            case '[': type = TokenType::LeftBracket; break;
            case ']': type = TokenType::RightBracket; break;

            case ';': type = TokenType::Semicolon; break;
            case ',': type = TokenType::Comma; break;
            case '.': type = TokenType::Dot; break;

            default:
                // Preserve the invalid character and report its location.
                tokens.push_back({
                    TokenType::Unknown,
                    std::string(1, c),
                    tokenLine,
                    tokenColumn
                });

                addError(
                    std::string("Invalid character '") + c + "'",
                    tokenLine,
                    tokenColumn
                );

                advance();
                continue;
        }

        tokens.push_back({
            type,
            std::string(1, c),
            tokenLine,
            tokenColumn
        });

        advance();
    }

    // Append EOF so the parser can detect the end of the input.
    tokens.push_back({
        TokenType::EndOfFile,
        "",
        line,
        column
    });

    return tokens;
}