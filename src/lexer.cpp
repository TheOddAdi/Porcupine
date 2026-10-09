
#include "lexer.hpp"

#include <cctype>
#include <string>

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
    char c = current();

    if (c != '\0') {
        ++position;

        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }

    return c;
}

// Skips spaces, tabs, newlines, and other whitespace.
void Lexer::skipWhitespace() {
    while (std::isspace(
        static_cast<unsigned char>(current()))) {
        advance();
    }
}

// Skips a // comment until the end of the line.
void Lexer::skipLineComment() {
    while (current() != '\0' && current() != '\n') {
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

    while (current() != '\0') {
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

    while (current() != '\0') {
        skipWhitespace();

        if (current() == '\0') {
            break;
        }

        // Comments must be recognized before treating / as an operator.
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
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string text;

            while (std::isalnum(
                       static_cast<unsigned char>(current())) ||
                   current() == '_') {
                text += advance();
            }

            TokenType type = TokenType::Identifier;

            if (text == "int") type = TokenType::KeywordInt;
            else if (text == "float") type = TokenType::KeywordFloat;
            else if (text == "bool") type = TokenType::KeywordBool;
            else if (text == "char") type = TokenType::KeywordChar;
            else if (text == "void") type = TokenType::KeywordVoid;
            else if (text == "string") type = TokenType::KeywordString;
            else if (text == "return") type = TokenType::KeywordReturn;
            else if (text == "if") type = TokenType::KeywordIf;
            else if (text == "else") type = TokenType::KeywordElse;
            else if (text == "while") type = TokenType::KeywordWhile;

            tokens.push_back({type, text, tokenLine, tokenColumn});
            continue;
        }

        // NUMERIC LITERALS
        if (std::isdigit(static_cast<unsigned char>(c))) {
            std::string text;

            // Read the integer portion.
            while (std::isdigit(
                static_cast<unsigned char>(current()))) {
                text += advance();
            }

            // A decimal point followed by a digit makes this a float.
            // This keeps a.b and 123.member from being treated as floats.
            if (current() == '.' &&
                std::isdigit(static_cast<unsigned char>(peek()))) {
                text += advance();

                while (std::isdigit(
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

            // Include the opening quote in the token's original text.
            text += advance();

            while (current() != '\0') {
                if (current() == '"') {
                    text += advance();
                    terminated = true;
                    break;
                }

                // A raw newline is not allowed inside this string syntax.
                if (current() == '\n') {
                    break;
                }

                if (current() == '\\') {
                    // Preserve the backslash and the escaped character.
                    text += advance();

                    if (current() == '\0' || current() == '\n') {
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
        // Check the longest operators first, such as <<= and >>=.
        if (c == '<' && peek() == '<' && peekNext() == '=') {
            advance();
            advance();
            advance();
            tokens.push_back({
                TokenType::LeftShiftEquals, "<<=", tokenLine, tokenColumn
            });
            continue;
        }

        if (c == '>' && peek() == '>' && peekNext() == '=') {
            advance();
            advance();
            advance();
            tokens.push_back({
                TokenType::RightShiftEquals, ">>=", tokenLine, tokenColumn
            });
            continue;
        }

        // Two-character operators.
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
                tokens.push_back({type, op, tokenLine, tokenColumn});
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
                // Preserve the bad character as a token and report it.
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

    // Always append EOF so the parser can detect the end of the input.
    tokens.push_back({
        TokenType::EndOfFile,
        "",
        line,
        column
    });

    return tokens;
}