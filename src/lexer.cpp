#include <string>
#include <vector>
#include <cctype>
#include "lexer.hpp"


char Lexer::advance() {
    char c;
    if (currIndex >= sourceCode.length()) {
        c = '\0';
    } else {
        c = sourceCode[currIndex];
        currIndex++;
        if (c == '\n') {
            currLine++;
            currColumn = 1;
        } else {
            currColumn += 1;
        }
    }
    return c;
}

char Lexer::peek() {
    char c;
    if (currIndex >= sourceCode.length()) {
        c = '\0';
    } else {
        c = sourceCode[currIndex];
    }
    return c;
}

char Lexer::peekNext() {
    char c;
    if (currIndex + 1 >= sourceCode.length()) {
        c = '\0';
    } else {
        c = sourceCode[currIndex + 1];
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (std::isspace(static_cast<unsigned char>(peek()))) {
        advance();
    }
}

void Lexer::skipComments() {
    char c;
    if (peek() == '/' && peekNext() == '/') {
        c = advance();
        c = advance();
        while (c != '\n' && c != '\0') {
            c = advance();
        }
    } else if (peek() == '/' && peekNext() == '*') {
        advance();
        advance();
        bool commentEndFound = false;
        while (!commentEndFound) {
            if (peek() == '\0') {
                break;
            }
            if (peek() == '*' && peekNext() == '/') {
                commentEndFound = true;
                advance();
                advance();
            } else {
                advance();
            }
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (peek() != '\0') {
        while (std::isspace(static_cast<unsigned char>(peek())) || (peek() == '/' && (peekNext() == '/' || peekNext() == '*'))) {
            skipWhitespace();
            skipComments();
        }
    }
    tokens.push_back(Token{TokenKind::EOFToken, "", currLine, currColumn});
}

/*
    Lexer(std::string code) : sourceCode(code) {

    }
    std::vector<Token> tokenize();
*/