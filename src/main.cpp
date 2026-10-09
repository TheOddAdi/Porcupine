#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

void printExpression(const Expr* expression, int indent);
void printStatement(const Stmt* statement, int indent);

void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

void printExpression(const Expr* expression, int indent) {
    if (!expression) {
        return;
    }

    printIndent(indent);

    if (auto integer = dynamic_cast<const IntExpr*>(expression)) {
        std::cout << "IntExpr: " << integer->value << '\n';
    }
    else if (auto variable =
                 dynamic_cast<const VariableExpr*>(expression)) {
        std::cout << "VariableExpr: " << variable->name << '\n';
    }
    else if (auto binary = dynamic_cast<const BinaryExpr*>(expression)) {
        std::cout << "BinaryExpr: " << binary->op << '\n';
        printExpression(binary->left.get(), indent + 1);
        printExpression(binary->right.get(), indent + 1);
    }
    else if (auto call = dynamic_cast<const CallExpr*>(expression)) {
        std::cout << "CallExpr: " << call->name << '\n';

        for (const auto& argument : call->arguments) {
            printExpression(argument.get(), indent + 1);
        }
    }
}

void printStatement(const Stmt* statement, int indent) {
    if (!statement) {
        return;
    }

    printIndent(indent);

    if (auto declaration = dynamic_cast<const VarDeclStmt*>(statement)) {
        std::cout << "VarDeclStmt: "
                  << declaration->typeName << ' '
                  << declaration->name << '\n';

        if (declaration->initializer) {
            printExpression(declaration->initializer.get(), indent + 1);
        }
    }
    else if (auto ret = dynamic_cast<const ReturnStmt*>(statement)) {
        std::cout << "ReturnStmt\n";
        printExpression(ret->value.get(), indent + 1);
    }
    else if (auto expr = dynamic_cast<const ExprStmt*>(statement)) {
        std::cout << "ExprStmt\n";
        printExpression(expr->expression.get(), indent + 1);
    }
    else if (auto block = dynamic_cast<const BlockStmt*>(statement)) {
        std::cout << "BlockStmt\n";

        for (const auto& child : block->statements) {
            printStatement(child.get(), indent + 1);
        }
    }
}

void printProgram(const Program& program) {
    std::cout << "Program\n";

    for (const auto& function : program.functions) {
        std::cout << "  FunctionDecl: "
                  << function.returnType << ' '
                  << function.name << '\n';

        for (const auto& parameter : function.parameters) {
            std::cout << "    Parameter: "
                      << parameter.typeName << ' '
                      << parameter.name << '\n';
        }

        printStatement(function.body.get(), 2);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: porcupine <source.pp>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);

    if (!file) {
        std::cerr << "Error: Could not open file: "
                  << argv[1] << '\n';
        return 1;
    }

    std::string source{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    bool failed = false;

    for (const auto& error : lexer.getErrors()) {
        std::cerr << argv[1] << ':'
                  << error.line << ':' << error.column
                  << ": lexer error: " << error.message << '\n';
        failed = true;
    }

    if (failed) {
        return 1;
    }

    Parser parser(tokens);
    std::unique_ptr<Program> program = parser.parse();

    for (const auto& error : parser.getErrors()) {
        std::cerr << argv[1] << ':'
                  << error.line << ':' << error.column
                  << ": parser error: " << error.message << '\n';
        failed = true;
    }

    if (failed) {
        return 1;
    }

    printProgram(*program);
    return 0;
}