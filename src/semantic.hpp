#pragma once

#include "ast.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct SemanticError {
    std::string message;
};

class SemanticAnalyzer {
public:
    void analyze(const Program& program);

    const std::vector<SemanticError>& getErrors() const;

private:
    struct FunctionInfo {
        std::string returnType;
        std::vector<std::string> parameterTypes;
    };

    std::unordered_map<std::string, FunctionInfo> functions;
    std::vector<std::unordered_set<std::string>> scopes;
    std::vector<SemanticError> errors;

    void addError(const std::string& message);

    void analyzeFunction(const FunctionDecl& function);
    void analyzeStatement(const Stmt* statement);
    void analyzeExpression(const Expr* expression);

    bool isVariableDeclared(const std::string& name) const;
    bool isKnownType(const std::string& typeName) const;

    void enterScope();
    void leaveScope();
};