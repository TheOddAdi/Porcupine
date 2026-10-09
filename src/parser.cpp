#include "parser.hpp"

#include <utility>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens) {}

const std::vector<ParserError>& Parser::getErrors() const {
    return errors;
}

const Token& Parser::current() const {
    return tokens[position];
}

const Token& Parser::peek(std::size_t offset) const {
    std::size_t index = position + offset;

    if (index >= tokens.size()) {
        return tokens.back();
    }

    return tokens[index];
}

const Token& Parser::advance() {
    if (!check(TokenType::EndOfFile)) {
        ++position;
    }

    return tokens[position - 1];
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }

    advance();
    return true;
}

bool Parser::expect(TokenType type, const std::string& message) {
    if (match(type)) {
        return true;
    }

    addError(message);
    return false;
}

void Parser::addError(const std::string& message) {
    errors.push_back({
        message,
        current().line,
        current().column
    });
}

// Parse the entire program.
std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();

    while (!check(TokenType::EndOfFile)) {
        std::size_t previousPosition = position;

        auto function = parseFunction();

        if (function) {
            program->functions.push_back(std::move(*function));
        }

        // Prevent an infinite loop if parsing fails.
        if (position == previousPosition) {
            advance();
        }
    }

    return program;
}

// Parse a function declaration.
std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    auto function = std::make_unique<FunctionDecl>();

    if (!check(TokenType::KeywordInt) &&
        !check(TokenType::KeywordFloat) &&
        !check(TokenType::KeywordBool) &&
        !check(TokenType::KeywordChar) &&
        !check(TokenType::KeywordVoid) &&
        !check(TokenType::KeywordString) &&
        !check(TokenType::Identifier)) {
        addError("Expected a return type.");
        return nullptr;
    }

    function->returnType = advance().text;

    if (!expect(TokenType::Identifier, "Expected a function name.")) {
        return nullptr;
    }

    function->name = tokens[position - 1].text;

    if (!expect(TokenType::LeftParen, "Expected '(' after function name.")) {
        return nullptr;
    }

    // Parse parameters, such as int a, float b.
    if (!check(TokenType::RightParen)) {
        do {
            if (!check(TokenType::KeywordInt) &&
                !check(TokenType::KeywordFloat) &&
                !check(TokenType::KeywordBool) &&
                !check(TokenType::KeywordChar) &&
                !check(TokenType::KeywordString) &&
                !check(TokenType::Identifier)) {
                addError("Expected a parameter type.");
                return nullptr;
            }

            Parameter parameter;
            parameter.typeName = advance().text;

            if (!expect(TokenType::Identifier, "Expected a parameter name.")) {
                return nullptr;
            }

            parameter.name = tokens[position - 1].text;
            function->parameters.push_back(std::move(parameter));

        } while (match(TokenType::Comma));
    }

    if (!expect(TokenType::RightParen, "Expected ')' after parameters.")) {
        return nullptr;
    }

    function->body = parseBlock();

    if (!function->body) {
        return nullptr;
    }

    return function;
}

// Parse a block enclosed in braces.
std::unique_ptr<BlockStmt> Parser::parseBlock() {
    if (!expect(TokenType::LeftBrace, "Expected '{' to begin a block.")) {
        return nullptr;
    }

    auto block = std::make_unique<BlockStmt>();

    while (!check(TokenType::RightBrace) &&
           !check(TokenType::EndOfFile)) {
        std::size_t previousPosition = position;

        auto statement = parseStatement();

        if (statement) {
            block->statements.push_back(std::move(statement));
        }

        if (position == previousPosition) {
            advance();
        }
    }

    if (!expect(TokenType::RightBrace, "Expected '}' to end a block.")) {
        return nullptr;
    }

    return block;
}

// Parse one statement.
std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::KeywordReturn)) {
        return parseReturnStatement();
    }

    if (check(TokenType::KeywordInt) ||
        check(TokenType::KeywordFloat) ||
        check(TokenType::KeywordBool) ||
        check(TokenType::KeywordChar) ||
        check(TokenType::KeywordString) ||
        check(TokenType::Identifier)) {
        // An identifier followed by '(' is a function call,
        // not a variable declaration.
        if (check(TokenType::Identifier) &&
            peek().type == TokenType::LeftParen) {
            return parseExpressionStatement();
        }

        return parseVariableDeclaration();
    }

    return parseExpressionStatement();
}

// Parse a declaration, such as: int result = 10;
std::unique_ptr<Stmt> Parser::parseVariableDeclaration() {
    auto declaration = std::make_unique<VarDeclStmt>();

    declaration->typeName = advance().text;

    if (!expect(TokenType::Identifier, "Expected a variable name.")) {
        return nullptr;
    }

    declaration->name = tokens[position - 1].text;

    if (match(TokenType::Equals)) {
        declaration->initializer = parseExpression();

        if (!declaration->initializer) {
            return nullptr;
        }
    }

    if (!expect(TokenType::Semicolon, "Expected ';' after variable declaration.")) {
        return nullptr;
    }

    return declaration;
}

// Parse: return expression;
std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    advance(); // Consume 'return'.

    auto statement = std::make_unique<ReturnStmt>();

    if (!check(TokenType::Semicolon)) {
        statement->value = parseExpression();

        if (!statement->value) {
            return nullptr;
        }
    }

    if (!expect(TokenType::Semicolon, "Expected ';' after return statement.")) {
        return nullptr;
    }

    return statement;
}

// Parse an expression followed by a semicolon.
std::unique_ptr<Stmt> Parser::parseExpressionStatement() {
    auto statement = std::make_unique<ExprStmt>();

    statement->expression = parseExpression();

    if (!statement->expression) {
        return nullptr;
    }

    if (!expect(TokenType::Semicolon, "Expected ';' after expression.")) {
        return nullptr;
    }

    return statement;
}

// Addition and subtraction have lower precedence than multiplication.
std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAdditive();
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto expression = parseMultiplicative();

    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        std::string op = advance().text;
        auto right = parseMultiplicative();

        if (!right) {
            return nullptr;
        }

        expression = std::make_unique<BinaryExpr>(
            op,
            std::move(expression),
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto expression = parsePrimary();

    while (check(TokenType::Star) ||
           check(TokenType::Slash) ||
           check(TokenType::Percent)) {
        std::string op = advance().text;
        auto right = parsePrimary();

        if (!right) {
            return nullptr;
        }

        expression = std::make_unique<BinaryExpr>(
            op,
            std::move(expression),
            std::move(right)
        );
    }

    return expression;
}

// Parse literals, variables, parenthesized expressions, and function calls.
std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::IntegerLiteral)) {
        const Token& token = tokens[position - 1];

        try {
            return std::make_unique<IntExpr>(std::stoi(token.text));
        } catch (...) {
            addError("Integer literal is outside the supported range.");
            return nullptr;
        }
    }

    if (match(TokenType::Identifier)) {
        std::string name = tokens[position - 1].text;

        if (match(TokenType::LeftParen)) {
            auto call = std::make_unique<CallExpr>(name);

            if (!check(TokenType::RightParen)) {
                do {
                    auto argument = parseExpression();

                    if (!argument) {
                        return nullptr;
                    }

                    call->arguments.push_back(std::move(argument));

                } while (match(TokenType::Comma));
            }

            if (!expect(TokenType::RightParen, "Expected ')' after arguments.")) {
                return nullptr;
            }

            return call;
        }

        return std::make_unique<VariableExpr>(name);
    }

    if (match(TokenType::LeftParen)) {
        auto expression = parseExpression();

        if (!expect(TokenType::RightParen, "Expected ')' after expression.")) {
            return nullptr;
        }

        return expression;
    }

    addError("Expected an expression.");
    return nullptr;
}