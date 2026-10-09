#pragma once

#include "ast.hpp"
#include "lexer.hpp"

#include <memory>
#include <string>
#include <vector>

struct ParserError {
    std::string message;
    int line;
    int column;
};

class Parser {
    public:
        explicit Parser(const std::vector<Token>& tokens);

        std::unique_ptr<Program> parse();

        const std::vector<ParserError>& getErrors() const;

    private:
        const std::vector<Token>& tokens;
        std::size_t position = 0;
    
        std::vector<ParserError> errors;
    
        const Token& current() const;
        const Token& peek(std::size_t offset = 1) const;
        const Token& advance();
        bool check(TokenType type) const;
        bool match(TokenType type);
        bool expect(TokenType type, const std::string& message);
        void addError(const std::string& message);
    
        std::unique_ptr<FunctionDecl> parseFunction();
        std::unique_ptr<BlockStmt> parseBlock();
        std::unique_ptr<Stmt> parseStatement();
        std::unique_ptr<Stmt> parseVariableDeclaration();
        std::unique_ptr<Stmt> parseReturnStatement();
        std::unique_ptr<Stmt> parseExpressionStatement();
    
        std::unique_ptr<Expr> parseExpression();
        std::unique_ptr<Expr> parseAdditive();
        std::unique_ptr<Expr> parseMultiplicative();
        std::unique_ptr<Expr> parsePrimary();

};
