
#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // Require a source filename as a command-line argument.
    if (argc < 2) {
        std::cerr << "Usage: porcupine <source.pp>\n";
        return 1;
    }

    // Open the Porcupine source file.
    std::ifstream file(argv[1]);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: "
                  << argv[1] << '\n';
        return 1;
    }

    // Read the entire file into memory.
    std::string source{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    // Convert the source into tokens.
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // Print every token except the end-of-file marker.
    for (const Token& token : tokens) {
        if (token.type == TokenType::EndOfFile) {
            continue;
        }

        std::cout << token.line << ':' << token.column
                  << "  " << token.text << '\n';
    }

    // Display any lexical errors.
    const std::vector<LexerError>& errors = lexer.getErrors();

    for (const LexerError& error : errors) {
        std::cerr << argv[1] << ':'
                  << error.line << ':'
                  << error.column << ": error: "
                  << error.message << '\n';
    }

    // Return a nonzero exit code if the source contains errors.
    return errors.empty() ? 0 : 1;
}