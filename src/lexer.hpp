#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Every kind of token the lexer can recognize.
enum class TokenType {
    // Built-in type keywords
    KeywordInt,
    KeywordFloat,
    KeywordBool,
    KeywordChar,
    KeywordVoid,
    KeywordString,
    KeywordUnsigned,
    KeywordVar,

    // Declaration keywords
    KeywordConst,
    KeywordType,

    // Control flow
    KeywordReturn,
    KeywordIf,
    KeywordElse,
    KeywordWhile,
    KeywordFor,
    KeywordDo,
    KeywordBreak,
    KeywordContinue,

    // Comparison statement
    KeywordCompare,
    KeywordCase,
    KeywordDefault,

    // Special keywords and boolean literals
    KeywordNullptr,
    KeywordTrue,
    KeywordFalse,

    // Names and literals
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,

    // Arithmetic operators
    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %

    // Increment, decrement, and compound assignment
    PlusPlus,      // ++
    MinusMinus,    // --
    PlusEquals,    // +=
    MinusEquals,   // -=
    StarEquals,    // *=
    SlashEquals,   // /=
    PercentEquals, // %=

    // Comparison and assignment
    Equals,        // =
    EqualEqual,    // ==
    Bang,          // !
    BangEqual,     // !=
    Less,          // <
    Greater,       // >
    LessEqual,     // <=
    GreaterEqual,  // >=

    // Bitwise and logical operators
    Ampersand,        // &
    Pipe,             // |
    Caret,            // ^
    Tilde,            // ~
    AmpAmp,           // &&
    PipePipe,         // ||
    LeftShift,        // <<
    RightShift,       // >>
    AmpEquals,        // &=
    PipeEquals,       // |=
    CaretEquals,      // ^=
    LeftShiftEquals,  // <<=
    RightShiftEquals, // >>=

    // Pointer and other operators
    Arrow,         // ->
    Question,      // ?
    Colon,         // :

    // Punctuation
    LeftParen,     // (
    RightParen,    // )
    LeftBrace,     // {
    RightBrace,    // }
    LeftBracket,   // [
    RightBracket,  // ]
    Semicolon,     // ;
    Comma,         // ,
    Dot,           // .

    // Special
    EndOfFile,
    Unknown
};

// A token's type, source text, and starting position.
struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
};

// A lexer error with its source location.
struct LexerError {
    std::string message;
    int line;
    int column;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);

    // Converts source code into a list of tokens.
    std::vector<Token> tokenize();

    // Returns errors found during tokenization.
    const std::vector<LexerError>& getErrors() const;

private:
    const std::string& source;

    std::size_t position = 0;
    int line = 1;
    int column = 1;

    std::vector<LexerError> errors;

    char current() const;
    char peek() const;
    char peekNext() const;
    char advance();

    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();

    void addError(
        const std::string& message,
        int errorLine,
        int errorColumn
    );
};