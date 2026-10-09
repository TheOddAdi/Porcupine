#include "semantic.hpp"

#include <utility>

void SemanticAnalyzer::addError(const std::string& message) {
    errors.push_back({message});
}

const std::vector<SemanticError>&
SemanticAnalyzer::getErrors() const {
    return errors;
}

bool SemanticAnalyzer::isKnownType(
    const std::string& typeName
) const {
    return typeName == "int" ||
           typeName == "float" ||
           typeName == "bool" ||
           typeName == "char" ||
           typeName == "void" ||
           typeName == "string";
}

void SemanticAnalyzer::enterScope() {
    scopes.emplace_back();
}

void SemanticAnalyzer::leaveScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

bool SemanticAnalyzer::isVariableDeclared(
    const std::string& name
) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }

    return false;
}

void SemanticAnalyzer::analyze(const Program& program) {
    errors.clear();
    functions.clear();
    scopes.clear();

    // First pass: collect function declarations.
    // This allows a function to call another function
    // declared later in the source file.
    for (const auto& function : program.functions) {
        if (functions.find(function.name) != functions.end()) {
            addError(
                "Function '" + function.name +
                "' is already declared."
            );
            continue;
        }

        FunctionInfo info;
        info.returnType = function.returnType;

        for (const auto& parameter : function.parameters) {
            info.parameterTypes.push_back(parameter.typeName);
        }

        functions.emplace(function.name, std::move(info));
    }

    // Second pass: analyze function bodies.
    for (const auto& function : program.functions) {
        analyzeFunction(function);
    }

    // A Porcupine executable must have an entry point.
    if (functions.find("main") == functions.end()) {
        addError("Program is missing the main function.");
    }
}

void SemanticAnalyzer::analyzeFunction(
    const FunctionDecl& function
) {
    enterScope();

    // Parameters belong to the function's outer scope.
    for (const auto& parameter : function.parameters) {
        if (!isKnownType(parameter.typeName)) {
            addError(
                "Unknown parameter type '" +
                parameter.typeName + "' in function '" +
                function.name + "'."
            );
        }

        if (isVariableDeclared(parameter.name)) {
            addError(
                "Duplicate parameter '" +
                parameter.name + "' in function '" +
                function.name + "'."
            );
        } else {
            scopes.back().insert(parameter.name);
        }
    }

    analyzeStatement(function.body.get());

    leaveScope();
}

void SemanticAnalyzer::analyzeStatement(
    const Stmt* statement
) {
    if (!statement) {
        return;
    }

    if (auto declaration =
            dynamic_cast<const VarDeclStmt*>(statement)) {

        if (!isKnownType(declaration->typeName)) {
            addError(
                "Unknown type '" + declaration->typeName +
                "' for variable '" + declaration->name + "'."
            );
        }

        // Analyze the initializer before declaring the variable.
        // This prevents a variable from referring to itself
        // during its own initialization.
        if (declaration->initializer) {
            analyzeExpression(declaration->initializer.get());
        }

        if (scopes.back().find(declaration->name) != scopes.back().end()) {
            addError(
                "Variable '" + declaration->name +
                "' is already declared in this scope."
            );
        } else {
            scopes.back().insert(declaration->name);
        }

        return;
    }

    if (auto ret = dynamic_cast<const ReturnStmt*>(statement)) {
        if (ret->value) {
            analyzeExpression(ret->value.get());
        }

        return;
    }

    if (auto expr = dynamic_cast<const ExprStmt*>(statement)) {
        analyzeExpression(expr->expression.get());
        return;
    }

    if (auto block = dynamic_cast<const BlockStmt*>(statement)) {
        // The function body shares the scope containing its
        // parameters. Nested blocks introduce new scopes.
        bool isFunctionBody =
            scopes.size() == 1;

        if (!isFunctionBody) {
            enterScope();
        }

        for (const auto& child : block->statements) {
            analyzeStatement(child.get());
        }

        if (!isFunctionBody) {
            leaveScope();
        }
    }
}

void SemanticAnalyzer::analyzeExpression(
    const Expr* expression
) {
    if (!expression) {
        return;
    }

    if (auto variable =
            dynamic_cast<const VariableExpr*>(expression)) {

        if (!isVariableDeclared(variable->name)) {
            addError(
                "Use of undeclared variable '" +
                variable->name + "'."
            );
        }

        return;
    }

    if (auto binary =
            dynamic_cast<const BinaryExpr*>(expression)) {

        analyzeExpression(binary->left.get());
        analyzeExpression(binary->right.get());
        return;
    }

    if (auto call =
            dynamic_cast<const CallExpr*>(expression)) {

        // Check the arguments even if the function is unknown.
        for (const auto& argument : call->arguments) {
            analyzeExpression(argument.get());
        }

        auto function = functions.find(call->name);

        if (function == functions.end()) {
            addError(
                "Call to undeclared function '" +
                call->name + "'."
            );
            return;
        }

        if (call->arguments.size() !=
            function->second.parameterTypes.size()) {

            addError(
                "Function '" + call->name +
                "' expects " +
                std::to_string(
                    function->second.parameterTypes.size()
                ) +
                " argument(s), but got " +
                std::to_string(call->arguments.size()) + "."
            );
        }

        return;
    }

    // IntExpr requires no semantic checks at this stage.
}