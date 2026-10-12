#pragma once
#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
private:
    std::string sourceCode;
    int currIndex = 0;
    int currLine = 1;
    int currColumn = 1;

    char advance();
    char peek();
    char peekNext();
    void skipWhitespace();
    void skipComments();

public:
    Lexer(std::string code) : sourceCode(code) {

    }
    std::vector<Token> tokenize();
};