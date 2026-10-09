#pragma once

#include <memory>
#include <string>
#include <vector>

// Base class for all expressions.
struct Expr {
    virtual ~Expr() = default;
};

// Integer literal, such as 42.
struct IntExpr : Expr {
    int value;

    explicit IntExpr(int value) : value(value) {}

};

// Variable reference, such as result.
struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(std::string name)
        : name(std::move(name)) {}

};

// Binary expression, such as a + b.
struct BinaryExpr : Expr {
        std::string op;
std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;


    BinaryExpr(
        std::string op,
        std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right
    ) : op(std::move(op)),
        left(std::move(left)),
        right(std::move(right)) {}

};

// Function call, such as add(10, 20).
struct CallExpr : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;

    explicit CallExpr(std::string name)
        : name(std::move(name)) {}

};

// Base class for all statements.
struct Stmt {
    virtual ~Stmt() = default;
};

// Variable declaration, such as int x = 10;
struct VarDeclStmt : Stmt {
    std::string typeName;
    std::string name;
    std::unique_ptr<Expr> initializer;
};

// Return statement, such as return x;
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
};

// Expression statement, such as add(1, 2);
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;
};

// A block containing statements, enclosed in braces.
struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

// Function parameter, such as int x.
struct Parameter {
    std::string typeName;
    std::string name;
};

// Function declaration.
struct FunctionDecl {
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::unique_ptr<BlockStmt> body;
};

// The entire source program.
struct Program {
    std::vector<FunctionDecl> functions;
};
