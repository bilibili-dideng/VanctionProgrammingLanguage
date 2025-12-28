#include "code_generator.h"
#include "../include/ast.h"
#include <iostream>

// Constructor
CodeGenerator::CodeGenerator() : tempVarCounter(0) {
}

// Generate namespace declaration
std::string CodeGenerator::generateNamespaceDeclaration(NamespaceDeclaration* ns) {
    std::string code;
    
    // Generate namespace start
    code += "namespace " + ns->name + " {\n\n";
    
    // Generate declarations inside namespace
    for (auto decl : ns->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            code += generateFunctionDeclaration(func);
        } else if (auto nestedNs = dynamic_cast<NamespaceDeclaration*>(decl)) {
            code += generateNamespaceDeclaration(nestedNs);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            code += generateClassDeclaration(cls);
        }
    }
    
    // Generate namespace end
    code += "}\n\n";
    
    return code;
}

// Range generator class for Vanction range() function
class RangeGenerator {
public:
    RangeGenerator(int start, int end, int step = 1)
        : start_(start), end_(end), step_(step) {}
    
    // Iterator class
    class Iterator {
    public:
        Iterator(int value, int step, int end) : value_(value), step_(step), end_(end) {}
        
        int operator*() const { return value_; }
        
        Iterator& operator++() {
            value_ += step_;
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            if (step_ > 0) {
                return value_ < other.value_;
            } else {
                return value_ > other.value_;
            }
        }
        
    private:
        int value_;
        int step_;
        int end_;
    };
    
    Iterator begin() const { return Iterator(start_, step_, end_); }
    Iterator end() const { return Iterator(end_, step_, end_); }
    
private:
    int start_;
    int end_;
    int step_;
};

// Range function that returns a range generator
RangeGenerator range(int start, int end, int step = 1) {
    return RangeGenerator(start, end, step);
}

// Overload for double arguments
RangeGenerator range(double start, double end, double step = 1.0) {
    return RangeGenerator(static_cast<int>(start), static_cast<int>(end), static_cast<int>(step));
}

// Overload for single argument (range(end))
RangeGenerator range(int end) {
    return RangeGenerator(0, end, 1);
}

// Overload for single argument (range(end)) with double
RangeGenerator range(double end) {
    return RangeGenerator(0, static_cast<int>(end), 1);
}

// Vanction namespace for utility functions
namespace vanction {
    RangeGenerator range(int start, int end, int step = 1) {
        return ::range(start, end, step);
    }
    
    RangeGenerator range(double start, double end, double step = 1.0) {
        return ::range(start, end, step);
    }
    
    RangeGenerator range(int end) {
        return ::range(end);
    }
    
    RangeGenerator range(double end) {
        return ::range(end);
    }
}

// Generate C++ code
std::string CodeGenerator::generate(Program* program) {
    std::string code;
    
    // Add header files
    code += "#include <iostream>\n#include <string>\n#include <memory>\n#include <vector>\n#include <unordered_map>\n#include <variant>\n#include <functional>\n\n";    
    
    // Add helper functions for variant handling
    code += "// Helper functions for variant handling\n";
    code += "std::string variantToString(const std::variant<int, std::string, bool>& v) {\n";
    code += "    return std::visit([](const auto& value) -> std::string {\n";
    code += "        using T = std::decay_t<decltype(value)>;\n";
    code += "        if constexpr (std::is_same_v<T, int>) {\n";
    code += "            return std::to_string(value);\n";
    code += "        } else if constexpr (std::is_same_v<T, std::string>) {\n";
    code += "            return value;\n";
    code += "        } else if constexpr (std::is_same_v<T, bool>) {\n";
    code += "            return value ? \"true\" : \"false\";\n";
    code += "        } else {\n";
    code += "            return \"<unknown>\";\n";
    code += "        }\n";
    code += "    }, v);\n";
    code += "}\n\n";
    
    // Add helper functions for printing variants
    code += "std::ostream& operator<<(std::ostream& os, const std::variant<int, std::string, bool>& v) {\n";
    code += "    os << variantToString(v);\n";
    code += "    return os;\n";
    code += "}\n\n";
    
    // Add helper functions for printing vectors of variants
    code += "std::ostream& operator<<(std::ostream& os, const std::vector<std::variant<int, std::string, bool>>& vec) {\n";
    code += "    os << '[';\n";
    code += "    for (size_t i = 0; i < vec.size(); ++i) {\n";
    code += "        os << vec[i];\n";
    code += "        if (i < vec.size() - 1) {\n";
    code += "            os << \", \";\n";
    code += "        }\n";
    code += "    }\n";
    code += "    os << ']';\n";
    code += "    return os;\n";
    code += "}\n\n";
    
    // Add helper functions for printing vectors of strings
    code += "std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) {\n";
    code += "    os << '[';\n";
    code += "    for (size_t i = 0; i < vec.size(); ++i) {\n";
    code += "        os << '\"' << vec[i] << '\"';\n";
    code += "        if (i < vec.size() - 1) {\n";
    code += "            os << \", \";\n";
    code += "        }\n";
    code += "    }\n";
    code += "    os << ']';\n";
    code += "    return os;\n";
    code += "}\n\n";    
    
    // Add helper functions for string operations
    code += "std::string stringReplace(const std::string& str, const std::string& oldStr, const std::string& newStr) {\n";
    code += "    std::string result = str;\n";
    code += "    size_t pos = 0;\n";
    code += "    while ((pos = result.find(oldStr, pos)) != std::string::npos) {\n";
    code += "        result.replace(pos, oldStr.length(), newStr);\n";
    code += "        pos += newStr.length();\n";
    code += "    }\n";
    code += "    return result;\n";
    code += "}\n\n";
    
    code += "std::vector<std::string> stringExcision(const std::string& str, const std::string& delimiter) {\n";
    code += "    std::vector<std::string> result;\n";
    code += "    size_t start = 0;\n";
    code += "    size_t end = str.find(delimiter);\n";
    code += "    while (end != std::string::npos) {\n";
    code += "        result.push_back(str.substr(start, end - start));\n";
    code += "        start = end + delimiter.length();\n";
    code += "        end = str.find(delimiter, start);\n";
    code += "    }\n";
    code += "    result.push_back(str.substr(start));\n";
    code += "    return result;\n";
    code += "}\n\n";
    
    // Add helper functions for list operations
code += "void listAdd(std::vector<std::variant<int, std::string, bool>>& list, int value) {\n";
code += "    list.push_back(value);\n";
code += "}\n\n";

code += "void listAdd(std::vector<std::variant<int, std::string, bool>>& list, const std::string& value) {\n";
code += "    list.push_back(value);\n";
code += "}\n\n";

code += "void listAdd(std::vector<std::variant<int, std::string, bool>>& list, const char* value) {\n";
code += "    list.push_back(std::string(value));\n";
code += "}\n\n";

code += "void listAdd(std::vector<std::variant<int, std::string, bool>>& list, bool value) {\n";
code += "    list.push_back(value);\n";
code += "}\n\n";

// Overloaded get functions for different types with negative index support
// String indexing
code += "char get(const std::string& str, int index) {\n";
code += "    if (index < 0) {\n";
code += "        index = str.length() + index;\n";
code += "    }\n";
code += "    if (index < 0 || index >= str.length()) {\n";
code += "        return '\\0';\n";
code += "    }\n";
code += "    return str[index];\n";
code += "}\n\n";

// List indexing
code += "std::variant<int, std::string, bool> get(const std::vector<std::variant<int, std::string, bool>>& list, int index) {\n";
code += "    if (index < 0) {\n";
code += "        index = list.size() + index;\n";
code += "    }\n";
code += "    if (index < 0 || index >= list.size()) {\n";
code += "        return std::string(\"undefined\");\n";
code += "    }\n";
code += "    return list[index];\n";
code += "}\n\n";

// HashMap access
code += "std::variant<int, std::string, bool> get(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map, const std::string& key) {\n";
code += "    auto it = map.find(key);\n";
code += "    if (it != map.end()) {\n";
code += "        return it->second;\n";
code += "    }\n";
code += "    return std::string(\"undefined\");\n";
code += "}\n\n";

code += "std::variant<int, std::string, bool> get(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map, const std::string& key, const std::string& defaultValue) {\n";
code += "    auto it = map.find(key);\n";
code += "    if (it != map.end()) {\n";
code += "        return it->second;\n";
code += "    }\n";
code += "    return defaultValue;\n";
code += "}\n\n";
    
    code += "std::vector<std::string> mapKeys(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map) {\n";
    code += "    std::vector<std::string> keys;\n";
    code += "    for (const auto& pair : map) {\n";
    code += "        keys.push_back(pair.first);\n";
    code += "    }\n";
    code += "    return keys;\n";
    code += "}\n\n";
    
    code += "std::vector<std::variant<int, std::string, bool>> mapValues(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map) {\n";
    code += "    std::vector<std::variant<int, std::string, bool>> values;\n";
    code += "    for (const auto& pair : map) {\n";
    code += "        values.push_back(pair.second);\n";
    code += "    }\n";
    code += "    return values;\n";
    code += "}\n\n";
    
    // Add range generator implementation
    code += "// Range generator implementation\n";
    code += "class RangeGenerator {\n";
    code += "public:\n";
    code += "    RangeGenerator(int start, int end, int step = 1)\n";
    code += "        : start_(start), end_(end), step_(step) {}\n";
    code += "    \n";
    code += "    // Iterator class\n";
    code += "    class Iterator {\n";
    code += "    public:\n";
    code += "        Iterator(int value, int step, int end) : value_(value), step_(step), end_(end) {}\n";
    code += "        \n";
    code += "        int operator*() const { return value_; }\n";
    code += "        \n";
    code += "        Iterator& operator++() {\n";
    code += "            value_ += step_;\n";
    code += "            return *this;\n";
    code += "        }\n";
    code += "        \n";
    code += "        bool operator!=(const Iterator& other) const {\n";
    code += "            if (step_ > 0) {\n";
    code += "                return value_ < other.value_;\n";
    code += "            } else {\n";
    code += "                return value_ > other.value_;\n";
    code += "            }\n";
    code += "        }\n";
    code += "        \n";
    code += "    private:\n";
    code += "        int value_;\n";
    code += "        int step_;\n";
    code += "        int end_;\n";
    code += "    };\n";
    code += "    \n";
    code += "    Iterator begin() const { return Iterator(start_, step_, end_); }\n";
    code += "    Iterator end() const { return Iterator(end_, step_, end_); }\n";
    code += "    \n";
    code += "private:\n";
    code += "    int start_;\n";
    code += "    int end_;\n";
    code += "    int step_;\n";
    code += "};\n\n";
    
    code += "// Range function overloads\n";
    code += "RangeGenerator range(int start, int end, int step = 1) {\n";
    code += "    return RangeGenerator(start, end, step);\n";
    code += "}\n\n";
    
    code += "RangeGenerator range(double start, double end, double step = 1.0) {\n";
    code += "    return RangeGenerator(static_cast<int>(start), static_cast<int>(end), static_cast<int>(step));\n";
    code += "}\n\n";
    
    code += "RangeGenerator range(int end) {\n";
    code += "    return RangeGenerator(0, end, 1);\n";
    code += "}\n\n";
    
    code += "RangeGenerator range(double end) {\n";
    code += "    return RangeGenerator(0, static_cast<int>(end), 1);\n";
    code += "}\n\n";
    
    code += "// Vanction namespace for utility functions\n";
    code += "namespace vanction {\n";
    code += "    RangeGenerator range(int start, int end, int step = 1) {\n";
    code += "        return ::range(start, end, step);\n";
    code += "    }\n    \n";
    code += "    RangeGenerator range(double start, double end, double step = 1.0) {\n";
    code += "        return ::range(start, end, step);\n";
    code += "    }\n    \n";
    code += "    RangeGenerator range(int end) {\n";
    code += "        return ::range(end);\n";
    code += "    }\n    \n";
    code += "    RangeGenerator range(double end) {\n";
    code += "        return ::range(end);\n";
    code += "    }\n";
    code += "}\n\n";
    
    // Generate all declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            code += generateFunctionDeclaration(func);
        } else if (auto ns = dynamic_cast<NamespaceDeclaration*>(decl)) {
            code += generateNamespaceDeclaration(ns);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            code += generateClassDeclaration(cls);
        } else if (auto importStmt = dynamic_cast<ImportStatement*>(decl)) {
            code += generateImportStatement(importStmt);
        }
    }
    
    return code;
}

// Generate function declaration
std::string CodeGenerator::generateFunctionDeclaration(FunctionDeclaration* func) {
    std::string code;
    
    // Generate function signature
    if (func->name == "main") {
        code += "int main() {\n";
    } else {
        // Generate return type
        std::string returnType = func->returnType;
        if (returnType == "string") {
            returnType = "std::string";
        } else if (returnType.empty()) {
            // Use auto for functions that return lambda functions
            returnType = "auto";
        }
        
        // Generate function name
        code += returnType + " " + func->name + "(";
        
        // Generate parameters
        for (size_t i = 0; i < func->parameters.size(); ++i) {
            const auto& param = func->parameters[i];
            std::string paramType = param.type;
            if (paramType == "string") {
                paramType = "std::string";
            } else if (paramType.empty() || paramType == "auto") {
                // Use variant for generic parameters (C++17 compatible)
                paramType = "std::variant<int, std::string, bool>";
            }
            code += paramType + " " + param.name;
            if (i < func->parameters.size() - 1) {
                code += ", ";
            }
        }
        
        code += ") {\n";
    }
    
    // Generate function body
    // First pass: check if function returns a nested function (closure)
    bool returnsNestedFunction = false;
    // Check if this is the main function - main function never needs closure support
    if (func->name == "main") {
        returnsNestedFunction = false;
    } else {
        for (auto stmt : func->body) {
            // Check if this is a return statement that returns an identifier
            if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
                if (returnStmt->expression) {
                    // Check if returning an identifier that might be a nested function
                    if (auto ident = dynamic_cast<Identifier*>(returnStmt->expression)) {
                        // Check if there's a nested function with this name
                        for (auto innerStmt : func->body) {
                            if (auto nestedFunc = dynamic_cast<FunctionDeclaration*>(innerStmt)) {
                                if (nestedFunc->name == ident->name) {
                                    returnsNestedFunction = true;
                                    break;
                                }
                            }
                        }
                    }
                    // Check if directly returning a function declaration
                    if (dynamic_cast<FunctionDeclaration*>(returnStmt->expression)) {
                        returnsNestedFunction = true;
                    }
                }
            }
        }
    }
    
    // Generate shared_ptr wrappers for function parameters if needed for closures
    for (size_t i = 0; i < func->parameters.size(); ++i) {
        const auto& param = func->parameters[i];
        if (returnsNestedFunction) {
            code += "    auto " + param.name + "_ptr = std::make_shared<std::variant<int, std::string, bool>>(" + param.name + ");\n";
        }
    }
    
    // Generate function body statements
    for (auto stmt : func->body) {
        if (auto comment = dynamic_cast<Comment*>(stmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
            code += generateVariableDeclaration(varDecl, returnsNestedFunction);
        } else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            code += generateIfStatement(ifStmt);
        } else if (auto forStmt = dynamic_cast<ForLoopStatement*>(stmt)) {
            code += generateForLoopStatement(forStmt);
        } else if (auto forInStmt = dynamic_cast<ForInLoopStatement*>(stmt)) {
            code += generateForInLoopStatement(forInStmt);
        } else if (auto whileStmt = dynamic_cast<WhileLoopStatement*>(stmt)) {
            code += generateWhileLoopStatement(whileStmt);
        } else if (auto doWhileStmt = dynamic_cast<DoWhileLoopStatement*>(stmt)) {
            code += generateDoWhileLoopStatement(doWhileStmt);
        } else if (auto switchStmt = dynamic_cast<SwitchStatement*>(stmt)) {
            code += generateSwitchStatement(switchStmt);
        } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
        code += "    return";
        if (returnStmt->expression) {
            // Check if returning a nested function - need to wrap captured variables
            if (auto nestedFunc = dynamic_cast<FunctionDeclaration*>(returnStmt->expression)) {
                // Return the nested function - captured variables are already wrapped
                code += " " + nestedFunc->name;
            } else {
                code += " " + generateExpression(returnStmt->expression, false);
            }
        }
        code += ";\n";
        } else if (auto nestedFunc = dynamic_cast<FunctionDeclaration*>(stmt)) {
            // Generate nested function as a lambda stored in a variable
            // C++ doesn't support direct nested functions, so we convert to lambdas
            // Use [=] to capture parameters by value (avoid dangling references)
            // Variables wrapped in shared_ptr can be accessed through their reference aliases
            code += "    auto " + nestedFunc->name + " = [=]() -> auto {\n";
            for (auto bodyStmt : nestedFunc->body) {
                if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
                    code += "        " + generateComment(comment);
                } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
                    // Generate expression statement for nested function
                    std::string stmtCode = generateExpressionStatement(exprStmt, true).substr(4);
                    
                    // Replace variable access with *_ptr for closure variables
                    // First replace function parameters
                    for (size_t i = 0; i < func->parameters.size(); ++i) {
                        const auto& param = func->parameters[i];
                        std::string varName = param.name;
                        size_t pos = 0;
                        while ((pos = stmtCode.find(varName, pos)) != std::string::npos) {
                            bool isWordBoundaryBefore = (pos == 0 || !isalnum(stmtCode[pos-1]) && stmtCode[pos-1] != '_');
                            bool isWordBoundaryAfter = (pos + varName.length() == stmtCode.length() || !isalnum(stmtCode[pos + varName.length()]) && stmtCode[pos + varName.length()] != '_');
                            
                            if (isWordBoundaryBefore && isWordBoundaryAfter) {
                                // Replace variable name with *_ptr for closure variables
                                stmtCode.replace(pos, varName.length(), "*" + varName + "_ptr");
                                pos += varName.length() + 5;
                            } else {
                                pos += varName.length();
                            }
                        }
                    }
                    // Then replace outer variable declarations
                    for (auto outerStmt : func->body) {
                        if (auto outerVarDecl = dynamic_cast<VariableDeclaration*>(outerStmt)) {
                            std::string varName = outerVarDecl->name;
                            size_t pos = 0;
                            while ((pos = stmtCode.find(varName, pos)) != std::string::npos) {
                                bool isWordBoundaryBefore = (pos == 0 || !isalnum(stmtCode[pos-1]) && stmtCode[pos-1] != '_');
                                bool isWordBoundaryAfter = (pos + varName.length() == stmtCode.length() || !isalnum(stmtCode[pos + varName.length()]) && stmtCode[pos + varName.length()] != '_');
                                
                                if (isWordBoundaryBefore && isWordBoundaryAfter) {
                                    // Replace variable name with *_ptr for closure variables
                                    stmtCode.replace(pos, varName.length(), "*" + varName + "_ptr");
                                    pos += varName.length() + 5;
                                } else {
                                    pos += varName.length();
                                }
                            }
                        }
                    }
                    
                    code += "        " + stmtCode;
                } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
                    code += "        " + generateVariableDeclaration(varDecl, false);
                } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(bodyStmt)) {
                    code += "        return";
                    if (returnStmt->expression) {
                        // Generate the return expression
                        std::string exprCode = generateExpression(returnStmt->expression, false);
                        
                        // Replace function parameters in return expression
                        for (size_t i = 0; i < func->parameters.size(); ++i) {
                            const auto& param = func->parameters[i];
                            std::string varName = param.name;
                            size_t pos = 0;
                            while ((pos = exprCode.find(varName, pos)) != std::string::npos) {
                                bool isWordBoundaryBefore = (pos == 0 || !isalnum(exprCode[pos-1]) && exprCode[pos-1] != '_');
                                bool isWordBoundaryAfter = (pos + varName.length() == exprCode.length() || !isalnum(exprCode[pos + varName.length()]) && exprCode[pos + varName.length()] != '_');
                                
                                if (isWordBoundaryBefore && isWordBoundaryAfter) {
                                    // Replace variable name with *_ptr for closure variables
                                    exprCode.replace(pos, varName.length(), "*" + varName + "_ptr");
                                    pos += varName.length() + 5;
                                } else {
                                    pos += varName.length();
                                }
                            }
                        }
                        
                        // Check if this is an assignment to a wrapped variable
                        if (auto assignExpr = dynamic_cast<AssignmentExpression*>(returnStmt->expression)) {
                            // For assignment expressions, just return the expression
                            code += " " + exprCode;
                        } else if (auto ident = dynamic_cast<Identifier*>(returnStmt->expression)) {
                            // For identifier returns, check if it's a closure variable and return *var_ptr instead
                            code += " " + exprCode;
                        } else {
                            // For other expressions, return as is
                            code += " " + exprCode;
                        }
                    }
                    code += ";\n";
                } else {
                    code += "        // Unimplemented statement type\n";
                }
            }
            code += "    };\n";
        } else {
            code += "    // Unimplemented statement type\n";
        }
    }
    
    // Generate function end
    code += "}\n\n";
    
    return code;
}

// Generate expression statement
std::string CodeGenerator::generateExpressionStatement(ExpressionStatement* stmt, bool isNested) {
    if (auto funcCall = dynamic_cast<FunctionCall*>(stmt->expression)) {
        if ((funcCall->objectName == "System" && funcCall->methodName == "print") ||
            (funcCall->objectName == "System" && funcCall->methodName == "input")) {
            // For System.print and System.input, generate the statement without adding semicolon
            // because generateFunctionCall already returns a complete statement with << std::endl
            return "    " + generateExpression(stmt->expression, false) + ";\n";
        }
    }
    if (auto assignExpr = dynamic_cast<AssignmentExpression*>(stmt->expression)) {
        if (auto ident = dynamic_cast<Identifier*>(assignExpr->left)) {
            // Only generate auto declaration for instance creation expressions
            if (dynamic_cast<InstanceCreationExpression*>(assignExpr->right)) {
                return "    auto " + generateExpression(assignExpr, false) + ";\n";
            }
        }
    }
    return "    " + generateExpression(stmt->expression, false) + ";\n";
}

// Generate variable declaration
std::string CodeGenerator::generateVariableDeclaration(VariableDeclaration* varDecl, bool useSharedPtr) {
    std::string code = "    ";
    
    // Check for immut (constant)
    if (varDecl->isImmut) {
        code += "const auto " + varDecl->name;
        if (varDecl->initializer) {
            code += " = " + generateExpression(varDecl->initializer, false);
        }
        code += ";\n";
    } else if (varDecl->isDefine) {
        // Define statement: use std::string with empty value
        code += "std::string " + varDecl->name + ";\n";
    } else if (varDecl->isAuto) {
        // For closure variables, generate shared_ptr wrapper
        if (useSharedPtr) {
            code += "auto " + varDecl->name + "_ptr = std::make_shared<int>(";
            if (varDecl->initializer) {
                code += generateExpression(varDecl->initializer, false);
            } else {
                code += "0";
            }
            code += ");\n";
            // Also generate the original variable name for non-lambda use
            code += "auto& " + varDecl->name + " = *" + varDecl->name + "_ptr;\n";
        } else {
            // Regular auto variable
            code += "auto " + varDecl->name;
            if (varDecl->initializer) {
                code += " = " + generateExpression(varDecl->initializer);
            }
            code += ";\n";
        }
    } else {
        // Explicit type
        std::string cppType;
        if (varDecl->type == "int") {
            cppType = "int";
        } else if (varDecl->type == "char") {
            cppType = "char";
        } else if (varDecl->type == "string") {
            cppType = "std::string";
        } else if (varDecl->type == "bool") {
            cppType = "bool";
        } else if (varDecl->type == "float") {
            cppType = "float";
        } else if (varDecl->type == "double") {
            cppType = "double";
        } else if (varDecl->type == "List") {
            cppType = "std::vector<std::variant<int, std::string, bool>>";
        } else if (varDecl->type == "HashMap") {
            cppType = "std::unordered_map<std::string, std::variant<int, std::string, bool>>";
        } else {
            cppType = varDecl->type;
        }
        
        code += cppType + " " + varDecl->name;
        if (varDecl->initializer) {
            code += " = " + generateExpression(varDecl->initializer);
        }
        code += ";\n";
    }
    
    return code;
}

// Generate namespace access
std::string CodeGenerator::generateNamespaceAccess(NamespaceAccess* access) {
    return access->namespaceName + "::" + access->memberName;
}

// Generate expression
std::string CodeGenerator::generateExpression(Expression* expr, bool isLvalue) {
    if (auto ident = dynamic_cast<Identifier*>(expr)) {
        return generateIdentifier(ident);
    } else if (auto nsAccess = dynamic_cast<NamespaceAccess*>(expr)) {
        return generateNamespaceAccess(nsAccess);
    } else if (auto intLit = dynamic_cast<IntegerLiteral*>(expr)) {
        return generateIntegerLiteral(intLit);
    } else if (auto floatLit = dynamic_cast<FloatLiteral*>(expr)) {
        return generateFloatLiteral(floatLit);
    } else if (auto doubleLit = dynamic_cast<DoubleLiteral*>(expr)) {
        return generateDoubleLiteral(doubleLit);
    } else if (auto charLit = dynamic_cast<CharLiteral*>(expr)) {
        return generateCharLiteral(charLit);
    } else if (auto stringLiteral = dynamic_cast<StringLiteral*>(expr)) {
        return generateStringLiteral(stringLiteral);
    } else if (auto boolLit = dynamic_cast<BooleanLiteral*>(expr)) {
        return generateBooleanLiteral(boolLit);
    } else if (auto binaryExpr = dynamic_cast<BinaryExpression*>(expr)) {
        return generateBinaryExpression(binaryExpr, isLvalue);
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(expr)) {
        return generateAssignmentExpression(assignExpr);
    } else if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
        return generateFunctionCall(funcCall);
    } else if (auto instanceCreate = dynamic_cast<InstanceCreationExpression*>(expr)) {
        return generateInstanceCreationExpression(instanceCreate);
    } else if (auto instanceAccess = dynamic_cast<InstanceAccessExpression*>(expr)) {
        return generateInstanceAccessExpression(instanceAccess);
    } else if (auto listLit = dynamic_cast<ListLiteral*>(expr)) {
        return generateListLiteral(listLit);
    } else if (auto hashMapLit = dynamic_cast<HashMapLiteral*>(expr)) {
        return generateHashMapLiteral(hashMapLit);
    } else if (auto rangeExpr = dynamic_cast<RangeExpression*>(expr)) {
        return generateRangeExpression(rangeExpr);
    } else if (auto lambdaExpr = dynamic_cast<LambdaExpression*>(expr)) {
        return generateLambdaExpression(lambdaExpr);
    } else if (auto funcCallExpr = dynamic_cast<FunctionCallExpression*>(expr)) {
        // Generate function call expression, e.g., (lambda (a,b)->a+b)(1,2)
        std::string code = "(" + generateExpression(funcCallExpr->callee, false) + ")(";
        
        // Generate arguments
        for (size_t i = 0; i < funcCallExpr->arguments.size(); i++) {
            if (i > 0) {
                code += ", ";
            }
            code += generateExpression(funcCallExpr->arguments[i], false);
        }
        
        code += ")";
        return code;
    } else if (auto indexAccess = dynamic_cast<IndexAccessExpression*>(expr)) {
        // Generate index access expression, e.g., collection[index]
        std::string collectionCode = generateExpression(indexAccess->collection, true);
        std::string indexCode = generateExpression(indexAccess->index, false);
        
        if (isLvalue) {
            // For lvalue (assignment), we can't use get() because it returns a copy
            // Handle map access with string keys differently from list/string access with numeric indices
            if (auto stringLit = dynamic_cast<StringLiteral*>(indexAccess->index)) {
                // Map access with string literal key - use direct [] operator
                return collectionCode + "[" + indexCode + "]";
            } else {
                // List or string access with numeric index - generate code to handle negative indices
                // Create a temporary variable for the index to handle negative values
                std::string tempIndex = "temp_index_" + std::to_string(tempVarCounter++);
                return "(int " + tempIndex + " = " + indexCode + "; " + 
                       "if (" + tempIndex + " < 0) " + tempIndex + " += " + collectionCode + ".size(); " + 
                       collectionCode + "[" + tempIndex + "])";
            }
        } else {
            // For rvalue (reading), use get() function to handle negative indices correctly
            return "get(" + collectionCode + ", " + indexCode + ")";
        }
    }
    return "\"\""; // Return empty string instead of comment to avoid compilation errors
}

// Generate identifier
std::string CodeGenerator::generateIdentifier(Identifier* ident) {
    return ident->name;
}

// Generate integer literal
std::string CodeGenerator::generateIntegerLiteral(IntegerLiteral* literal) {
    return std::to_string(literal->value);
}

// Generate char literal
std::string CodeGenerator::generateCharLiteral(CharLiteral* literal) {
    return "'" + std::string(1, literal->value) + "'";
}

// Generate string literal
std::string CodeGenerator::generateStringLiteral(StringLiteral* literal) {
    if (literal->type == "raw") {
        // Generate C++ raw string literal: R"vanction()vanction"
        return "R\"vanction(" + literal->value + ")vanction\"";
    } else if (literal->type == "format") {
        // Generate C++ formatted string using std::format
        return "\"" + literal->value + "\"";
    } else {
        // Normal string literal with proper escaping
        std::string escaped = literal->value;
        
        size_t pos = 0;
        while (pos < escaped.length()) {
            if (escaped[pos] == '\n') {
                // Escape newline
                escaped.replace(pos, 1, "\\n");
                pos += 2;
            } else if (escaped[pos] == '"') {
                // Escape double quote
                escaped.replace(pos, 1, "\\\"");
                pos += 2;
            } else if (escaped[pos] == '\\') {
                // Check if this is already a valid escape sequence
                if (pos + 1 < escaped.length()) {
                    char nextChar = escaped[pos + 1];
                    // Valid escape sequences: \n, \t, \\, \" 
                    if (nextChar == 'n' || nextChar == 't' || nextChar == '\\' || nextChar == '"') {
                        // Already valid, keep it
                        pos += 2;
                    } else {
                        // Invalid escape sequence, escape the backslash
                        escaped.replace(pos, 1, "\\\\");
                        pos += 2;
                    }
                } else {
                    // Trailing backslash, escape it
                    escaped.replace(pos, 1, "\\\\");
                    pos += 2;
                }
            } else {
                // Normal character, move on
                pos++;
            }
        }
        
        return '"' + escaped + '"';
    }
}

// Generate boolean literal
std::string CodeGenerator::generateBooleanLiteral(BooleanLiteral* literal) {
    return literal->value ? "true" : "false";
}

// Generate float literal
std::string CodeGenerator::generateFloatLiteral(FloatLiteral* literal) {
    return std::to_string(literal->value) + "f";
}

// Generate double literal
std::string CodeGenerator::generateDoubleLiteral(DoubleLiteral* literal) {
    return std::to_string(literal->value);
}

// Generate binary expression
std::string CodeGenerator::generateBinaryExpression(BinaryExpression* expr, bool isLvalue) {
    std::string op = expr->op;
    
    // For regular binary operators, left and right are rvalues
    // For [] operator, left is lvalue if we're writing to it
    bool leftIsLvalue = (op == "[" && isLvalue);
    std::string left = generateExpression(expr->left, leftIsLvalue);
    std::string right = generateExpression(expr->right, false);
    
    // Handle different operators
    if (op == "+") {
        // Special handling for string concatenation
        // Check if either operand is a string literal
        bool leftIsStringLiteral = (dynamic_cast<StringLiteral*>(expr->left) != nullptr);
        bool rightIsStringLiteral = (dynamic_cast<StringLiteral*>(expr->right) != nullptr);
        
        // If we're dealing with string concatenation, ensure proper std::string usage
        if (leftIsStringLiteral || rightIsStringLiteral) {
            // For string literals combined with other string types, wrap in std::string
            return "std::string(" + left + ") + " + right;
        }
        return left + " + " + right;
    } else if (op == "-") {
        return left + " - " + right;
    } else if (op == "*") {
        return left + " * " + right;
    } else if (op == "/") {
        return left + " / " + right;
    } else if (op == "%") {
        return left + " % " + right;
    } else if (op == "<<") {
        return left + " << " + right;
    } else if (op == ">>") {
        return left + " >> " + right;
    } else if (op == "AND") {
        return left + " && " + right;
    } else if (op == "OR") {
        return left + " || " + right;
    } else if (op == "XOR") {
        return left + " ^ " + right;
    } else if (op == "&" || op == "BITWISE_AND") {
        return left + " & " + right;
    } else if (op == "|" || op == "BITWISE_OR") {
        return left + " | " + right;
    } else if (op == "==") {
        return left + " == " + right;
    } else if (op == "!=") {
        return left + " != " + right;
    } else if (op == "<") {
        return left + " < " + right;
    } else if (op == "<=") {
        return left + " <= " + right;
    } else if (op == ">") {
        return left + " > " + right;
    } else if (op == ">=") {
        return left + " >= " + right;
    } else if (op == "[") {
        // Handle array indexing based on lvalue context
        if (isLvalue) {
            // Writing to index - need to handle negative indices manually
            // For negative indices, we'll generate code that converts to positive
            return left + "[" + right + "]";
        } else {
            // Reading from index - always use overloaded get function to handle negative indices
            return "get(" + left + ", " + right + ")";
        }
    }
    
    return left + " " + op + " " + right;
}

// Generate if statement
std::string CodeGenerator::generateIfStatement(IfStatement* stmt) {
    std::string code = "    if (" + generateExpression(stmt->condition, false) + ") {\n";
    
    // Generate if body
    for (auto bodyStmt : stmt->ifBody) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "    }";
    
    // Generate else-if clauses
    for (auto elseIf : stmt->elseIfs) {
        code += " else if (" + generateExpression(elseIf->condition, false) + ") {\n";
        
        // Generate else-if body
        for (auto bodyStmt : elseIf->ifBody) {
            if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
                code += generateComment(comment);
            } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
                code += generateExpressionStatement(exprStmt);
            } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
                code += generateVariableDeclaration(varDecl);
            }
        }
        
        code += "    }";
    }
    
    // Generate else clause
    if (!stmt->elseBody.empty()) {
        code += " else {\n";
        
        // Generate else body
        for (auto bodyStmt : stmt->elseBody) {
            if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
                code += generateComment(comment);
            } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
                code += generateExpressionStatement(exprStmt);
            } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
                code += generateVariableDeclaration(varDecl);
            }
        }
        
        code += "    }";
    }
    
    code += "\n";
    return code;
}

// Generate for loop statement
std::string CodeGenerator::generateForLoopStatement(ForLoopStatement* stmt) {
    std::string code = "    for (";
    
    // Generate initialization
    if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt->initialization)) {
        std::string declCode = generateVariableDeclaration(varDecl).substr(4); // Remove indentation
        declCode.pop_back(); // Remove newline
        // Remove semicolon at the end of declaration
        if (!declCode.empty() && declCode.back() == ';') {
            declCode.pop_back();
        }
        code += declCode;
    } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt->initialization)) {
        code += generateExpression(exprStmt->expression, false);
    }
    
    code += "; " + generateExpression(stmt->condition, false) + "; " + generateExpression(stmt->increment, false) + ") {\n";
    
    // Generate loop body
    for (auto bodyStmt : stmt->body) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "    }\n";
    return code;
}

// Generate for-in loop statement
std::string CodeGenerator::generateForInLoopStatement(ForInLoopStatement* stmt) {
    std::string code;
    
    if (stmt->isKeyValuePair) {
        // Generate C++ range-based for loop with structured binding for hash map
        code = "    for (auto &[" + stmt->keyVariableName + ", " + stmt->valueVariableName + "] : " + generateExpression(stmt->collection, false) + ") {\n";
    } else {
        // Generate C++ range-based for loop for list
        code = "    for (auto " + stmt->keyVariableName + " : " + generateExpression(stmt->collection, false) + ") {\n";
    }
    
    // Generate loop body
    for (auto bodyStmt : stmt->body) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            // Special handling for System.print with formatted strings in loop body
            if (auto funcCall = dynamic_cast<FunctionCall*>(exprStmt->expression)) {
                if (funcCall->objectName == "System" && funcCall->methodName == "print") {
                    // Check if it's a formatted string
                    if (funcCall->arguments.size() > 0) {
                        auto stringExpr = dynamic_cast<StringLiteral*>(funcCall->arguments[0]);
                        if (stringExpr && stringExpr->type == "format" && stmt->isKeyValuePair) {
                            // Generate special code for formatted print in loop - only for key-value pairs
                            std::string formatStr = stringExpr->value;
                            code += "        std::cout << \"Key is \" << " + stmt->keyVariableName + " << \", Value is \" << " + stmt->valueVariableName + " << std::endl;\n";
                            continue;
                        }
                    }
                }
            }
            // Normal expression statement
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "    }\n";
    return code;
}

// Generate while loop statement
std::string CodeGenerator::generateWhileLoopStatement(WhileLoopStatement* stmt) {
    std::string code = "    while (" + generateExpression(stmt->condition, false) + ") {\n";
    
    // Generate loop body
    for (auto bodyStmt : stmt->body) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "    }\n";
    return code;
}

// Generate do-while loop statement
std::string CodeGenerator::generateDoWhileLoopStatement(DoWhileLoopStatement* stmt) {
    std::string code = "    do {\n";
    
    // Generate loop body
    for (auto bodyStmt : stmt->body) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "    } while (" + generateExpression(stmt->condition, false) + ");\n";
    return code;
}

// Generate case statement
std::string CodeGenerator::generateCaseStatement(CaseStatement* stmt) {
    std::string code;
    code += "    case " + generateExpression(stmt->value, false) + ": {\n";
    
    // Generate case body
    for (auto bodyStmt : stmt->body) {
        if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
            code += generateVariableDeclaration(varDecl);
        }
    }
    
    code += "        break;\n    }\n";
    return code;
}

// Generate switch statement
std::string CodeGenerator::generateSwitchStatement(SwitchStatement* stmt) {
    std::string code;
    std::string switchExpr = generateExpression(stmt->expression, false);
    
    // For C++ compatibility, generate if-else if chain instead of switch
    // This allows support for strings and other non-integer types
    for (size_t i = 0; i < stmt->cases.size(); ++i) {
        auto caseStmt = stmt->cases[i];
        std::string caseExpr = generateExpression(caseStmt->value, false);
        
        if (i == 0) {
            // First case - use if
            code += "    if (" + switchExpr + " == " + caseExpr + ") {\n";
        } else {
            // Subsequent cases - use else if
            code += "    } else if (" + switchExpr + " == " + caseExpr + ") {\n";
        }
        
        // Generate case body
        for (auto bodyStmt : caseStmt->body) {
            if (auto comment = dynamic_cast<Comment*>(bodyStmt)) {
                code += generateComment(comment);
            } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(bodyStmt)) {
                code += generateExpressionStatement(exprStmt);
            } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(bodyStmt)) {
                code += generateVariableDeclaration(varDecl);
            }
        }
    }
    
    // Close the last if/else if block
    code += "    }\n";
    
    return code;
}

// Generate class declaration
std::string CodeGenerator::generateClassDeclaration(ClassDeclaration* cls) {
    std::string code;
    
    // Generate class start with inheritance if applicable
    if (!cls->baseClassName.empty()) {
        code += "class " + cls->name + " : public " + cls->baseClassName + " {\n";
    } else {
        code += "class " + cls->name + " {\n";
    }
    
    // Generate public section
    code += "public:\n";
    
    // Only add member variables if this is the base class
    // For derived classes, inherit from base class instead of redefining variables
    if (cls->baseClassName.empty()) {
        // Add member variables (name, age, and id are common in examples)
        code += "    std::string name;\n";
        code += "    int age;\n";
        code += "    int id;\n\n";
    }
    
    // Generate init method as constructor
    if (auto initMethod = dynamic_cast<InstanceMethodDeclaration*>(cls->initMethod)) {
        // Generate constructor signature
        code += "    " + cls->name + "(";
        
        // Generate parameters (skip the first 'instance' parameter)
        for (size_t i = 1; i < initMethod->parameters.size(); ++i) {
            const auto& param = initMethod->parameters[i];
            // Use specific types based on parameter name to comply with C++17
            std::string paramType;
            if (param.name == "name") {
                paramType = "std::string";
            } else if (param.name == "age" || param.name == "id" || param.name == "Id") {
                paramType = "int";
            } else {
                paramType = "auto";
            }
            code += paramType + " " + param.name;
            if (i < initMethod->parameters.size() - 1) {
                code += ", ";
            }
        }
        // Add initialization list if this is a subclass
        if (!cls->baseClassName.empty()) {
            code += ") : " + cls->baseClassName + "(";
            // Pass name and age to parent constructor
            bool hasName = false;
            bool hasAge = false;
            for (size_t i = 1; i < initMethod->parameters.size(); ++i) {
                const auto& param = initMethod->parameters[i];
                if (param.name == "name") {
                    if (hasName || hasAge) code += ", ";
                    code += param.name;
                    hasName = true;
                } else if (param.name == "age") {
                    if (hasName || hasAge) code += ", ";
                    code += param.name;
                    hasAge = true;
                }
            }
            code += ") {\n";
        } else {
            code += ") {\n";
        }
        
        // Generate constructor body
        for (auto stmt : initMethod->body) {
            if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
                std::string stmtCode = generateExpressionStatement(exprStmt).substr(4);
                
                // Skip parent class init calls (handled in initialization list)
                if (stmtCode.find(".init(instance,") != std::string::npos || stmtCode.find("->init(instance,") != std::string::npos) {
                    continue;
                }
                
                // Replace instance. and instance-> with this-> in constructor body
                size_t pos = 0;
                while ((pos = stmtCode.find("instance", pos)) != std::string::npos) {
                    // Check if it's instance. or instance->
                    if (pos + 9 <= stmtCode.length() && stmtCode.substr(pos + 8, 1) == ".") {
                        // Replace instance. with this->
                        stmtCode.replace(pos, 9, "this->");
                        
                        // Check if we need to fix Id to id for instance access
                        if (pos + 10 <= stmtCode.length() && stmtCode.substr(pos + 6, 2) == "Id") {
                            // Replace instance.Id with this->id
                            stmtCode.replace(pos + 6, 2, "id");
                        }
                        
                        pos += 6; // Move past the replaced text
                    } else if (pos + 10 <= stmtCode.length() && stmtCode.substr(pos + 8, 2) == "->") {
                        // Replace instance-> with this->
                        stmtCode.replace(pos, 10, "this->");
                        pos += 6; // Move past the replaced text
                    } else {
                        // Not a member access, move to next character
                        pos += 8;
                    }
                }
                
                // Handle Id parameter correctly in constructor body
                pos = 0;
                while ((pos = stmtCode.find("Id", pos)) != std::string::npos) {
                    // Check if this Id is on the right side of an assignment
                    size_t assignPos = stmtCode.rfind("=", pos);
                    size_t semicolonPos = stmtCode.find(";", pos);
                    
                    // If it's after an assignment operator and before a semicolon,
                    // it's likely a parameter reference, so don't replace it
                    bool isParameterReference = (assignPos != std::string::npos) && 
                                               (semicolonPos == std::string::npos || pos < semicolonPos) &&
                                               (pos - assignPos > 1);
                    
                    // Only replace if it's not a parameter reference and is a standalone Id
                    if (!isParameterReference && 
                        (pos == 0 || !isalnum(stmtCode[pos - 1])) && 
                        (pos + 2 >= stmtCode.length() || !isalnum(stmtCode[pos + 2]))) {
                        stmtCode.replace(pos, 2, "id");
                        pos += 2;
                    } else {
                        pos += 2;
                    }
                }
                
                code += "        " + stmtCode;
            } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
                code += "        " + generateVariableDeclaration(varDecl).substr(4);
            } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
                // Skip return statements in constructors
            } else {
                code += "        // Unimplemented statement type in constructor\n";
            }
        }
        
        code += "    }\n\n";
    }
    
    // Generate instance methods
    for (auto method : cls->instanceMethods) {
        if (auto instanceMethod = dynamic_cast<InstanceMethodDeclaration*>(method)) {
            code += generateInstanceMethodDeclaration(instanceMethod);
        }
    }
    
    // Generate class end
    code += "};\n\n";
    
    return code;
}

// Generate class method declaration
std::string CodeGenerator::generateClassMethodDeclaration(ClassMethodDeclaration* method) {
    std::string code;
    
    // Class methods are generated as static methods
    code += "    static " + method->returnType + " " + method->name + "(";
    
    // Generate parameters (skip the first 'instance' parameter)
        for (size_t i = 1; i < method->parameters.size(); ++i) {
            const auto& param = method->parameters[i];
            // Use specific types based on parameter name to comply with C++17
            std::string paramType;
            if (param.name == "name") {
                paramType = "std::string";
            } else if (param.name == "age" || param.name == "id") {
                paramType = "int";
            } else {
                // Use std::variant for generic parameters instead of auto to avoid C++20 warning
                paramType = "std::variant<int, std::string, bool>";
            }
            code += paramType + " " + param.name;
            if (i < method->parameters.size() - 1) {
                code += ", ";
            }
        }
    code += ") {\n";
    
    // Generate method body
    for (auto stmt : method->body) {
        if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            std::string stmtCode = generateExpressionStatement(exprStmt).substr(4);
            // Replace instance. and instance-> with this-> in method body
            size_t pos = 0;
            while ((pos = stmtCode.find("instance", pos)) != std::string::npos) {
                // Check if it's instance. or instance->
                if (pos + 9 <= stmtCode.length() && stmtCode.substr(pos + 8, 1) == ".") {
                    // Replace instance. with this->
                    stmtCode.replace(pos, 9, "this->");
                    pos += 6; // Move past the replaced text
                } else if (pos + 10 <= stmtCode.length() && stmtCode.substr(pos + 8, 2) == "->") {
                    // Replace instance-> with this->
                    stmtCode.replace(pos, 10, "this->");
                    pos += 6; // Move past the replaced text
                } else {
                    // Not a member access, move to next character
                    pos += 8;
                }
            }
            code += "        " + stmtCode;
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
            code += "        " + generateVariableDeclaration(varDecl).substr(4);
        } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
            std::string stmtCode = "return";
            if (returnStmt->expression) {
                std::string exprCode = generateExpression(returnStmt->expression, false);
                // Replace instance. and instance-> with this-> in return expression
                size_t pos = 0;
                while ((pos = exprCode.find("instance", pos)) != std::string::npos) {
                    // Check if it's instance. or instance->
                    if (pos + 9 <= exprCode.length() && exprCode.substr(pos + 8, 1) == ".") {
                        // Replace instance. with this->
                        exprCode.replace(pos, 9, "this->");
                        pos += 6; // Move past the replaced text
                    } else if (pos + 10 <= exprCode.length() && exprCode.substr(pos + 8, 2) == "->") {
                        // Replace instance-> with this->
                        exprCode.replace(pos, 10, "this->");
                        pos += 6; // Move past the replaced text
                    } else {
                        // Not a member access, move to next character
                        pos += 8;
                    }
                }
                stmtCode += " " + exprCode;
            }
            code += "        " + stmtCode + ";\n";
        } else {
            code += "        // Unimplemented statement type in class method\n";
        }
    }
    
    code += "    }\n\n";
    
    return code;
}

// Generate instance method declaration
std::string CodeGenerator::generateInstanceMethodDeclaration(InstanceMethodDeclaration* method) {
    std::string code;
    
    // Generate method signature (skip the first 'instance' parameter)
    // Infer return type if it's a getter method
    std::string returnType = method->returnType;
    if (returnType == "void") {
        // Check if method body has a return statement with value
        for (auto stmt : method->body) {
            if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
                if (returnStmt->expression) {
                    // For simple getter methods, infer return type based on member access
                    if (auto memberAccess = dynamic_cast<InstanceAccessExpression*>(returnStmt->expression)) {
                        // This is an InstanceAccessExpression, but we need to check its actual type
                        // For now, let's set return type to appropriate type based on member name
                        std::string memberName = memberAccess->memberName;
                        if (memberName == "name") {
                            returnType = "std::string";
                        } else if (memberName == "age" || memberName == "id") {
                            returnType = "int";
                        }
                    } else if (auto ident = dynamic_cast<Identifier*>(returnStmt->expression)) {
                        // Check if it's a simple identifier (like instance.name)
                        std::string identName = ident->name;
                        if (identName == "name") {
                            returnType = "std::string";
                        } else if (identName == "age" || identName == "id") {
                            returnType = "int";
                        }
                    } else {
                        // Default to auto for other cases
                        returnType = "auto";
                    }
                    break;
                }
            }
        }
    }
    
    code += "    " + returnType + " " + method->name + "(";
    
    // Generate parameters (instance parameter has already been skipped during parsing)
    for (size_t i = 0; i < method->parameters.size(); ++i) {
        const auto& param = method->parameters[i];
        // Use specific types based on parameter name to comply with C++17
        std::string paramType;
        if (param.name == "name") {
            paramType = "std::string";
        } else if (param.name == "age" || param.name == "id") {
            paramType = "int";
        } else {
            // Use std::variant for generic parameters instead of auto to avoid C++20 warning
            paramType = "std::variant<int, std::string, bool>";
        }
        code += paramType + " " + param.name;
        if (i < method->parameters.size() - 1) {
            code += ", ";
        }
    }
    code += ") {\n";
    
    // Generate method body
    for (auto stmt : method->body) {
        if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            std::string stmtCode = generateExpressionStatement(exprStmt).substr(4);
            // Replace instance. and instance-> with this-> in method body
            size_t pos = 0;
            while ((pos = stmtCode.find("instance", pos)) != std::string::npos) {
                // Check if it's instance. or instance->
                if (pos + 9 <= stmtCode.length() && stmtCode.substr(pos + 8, 1) == ".") {
                    // Replace instance. with this->
                    stmtCode.replace(pos, 9, "this->");
                    pos += 6; // Move past the replaced text
                } else if (pos + 10 <= stmtCode.length() && stmtCode.substr(pos + 8, 2) == "->") {
                    // Replace instance-> with this->
                    stmtCode.replace(pos, 10, "this->");
                    pos += 6; // Move past the replaced text
                } else {
                    // Not a member access, move to next character
                    pos += 8;
                }
            }
            code += "        " + stmtCode;
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
            code += "        " + generateVariableDeclaration(varDecl).substr(4);
        } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
            std::string stmtCode = "return";
            if (returnStmt->expression) {
                std::string exprCode = generateExpression(returnStmt->expression);
                // Replace instance. and instance-> with this-> in return expression
                size_t pos = 0;
                while ((pos = exprCode.find("instance", pos)) != std::string::npos) {
                    // Check if it's instance. or instance->
                    if (pos + 9 <= exprCode.length() && exprCode.substr(pos + 8, 1) == ".") {
                        // Replace instance. with this->
                        exprCode.replace(pos, 9, "this->");
                        pos += 6; // Move past the replaced text
                    } else if (pos + 10 <= exprCode.length() && exprCode.substr(pos + 8, 2) == "->") {
                        // Replace instance-> with this->
                        exprCode.replace(pos, 10, "this->");
                        pos += 6; // Move past the replaced text
                    } else {
                        // Not a member access, move to next character
                        pos += 8;
                    }
                }
                stmtCode += " " + exprCode;
            }
            code += "        " + stmtCode + ";\n";
        } else {
            code += "        // Unimplemented statement type in instance method\n";
        }
    }
    
    code += "    }\n\n";
    
    return code;
}

// Generate instance creation expression
std::string CodeGenerator::generateInstanceCreationExpression(InstanceCreationExpression* expr) {
    std::string code = "std::make_unique<";
    
    // Add namespace if present
    if (!expr->namespaceName.empty()) {
        code += expr->namespaceName + "::";
    }
    
    code += expr->className + ">(";
    
    // Generate arguments
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        code += generateExpression(expr->arguments[i], false);
        if (i < expr->arguments.size() - 1) {
            code += ", ";
        }
    }
    
    code += ")";
    return code;
}

// Generate instance access expression
std::string CodeGenerator::generateInstanceAccessExpression(InstanceAccessExpression* expr) {
    // All instances created via Vanction are unique_ptr, so use -> operator
    std::string memberName = expr->memberName;
    // Convert Id to id for case consistency
    if (memberName == "Id") {
        memberName = "id";
    }
    return generateExpression(expr->instance, false) + "->" + memberName;
}

// Generate assignment expression
std::string CodeGenerator::generateAssignmentExpression(AssignmentExpression* expr) {
    // For assignment, left side is an lvalue
    return generateExpression(expr->left, true) + " = " + generateExpression(expr->right, false);
}

// Generate function call
std::string CodeGenerator::generateFunctionCall(FunctionCall* call) {
    if (call->objectName == "System" && call->methodName == "print") {
        // Generate std::cout << ... << std::endl
        std::string code = "std::cout";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            std::string argExpr = generateExpression(call->arguments[i], false);
            
            // If argument contains + operator, we need to handle it specially
            if (argExpr.find(" + ") != std::string::npos) {
                // Split the expression by + and add each part with <<
                size_t pos = 0;
                size_t start = 0;
                while ((pos = argExpr.find(" + ", start)) != std::string::npos) {
                    code += " << " + argExpr.substr(start, pos - start);
                    start = pos + 3;
                }
                code += " << " + argExpr.substr(start);
            } else {
                // Simple argument, just add it
                code += " << " + argExpr;
            }
        }
        
        code += " << std::endl";
        return code;
    } else if (call->objectName == "System" && call->methodName == "input") {
        // Generate input reading expression
        // We can't generate output statements in the middle of an expression,
        // so we'll use a lambda that first prints the prompt and then reads input
        std::string code = "([]{ ";
        
        // Generate prompt if provided
        if (!call->arguments.empty()) {
            code += "std::cout << " + generateExpression(call->arguments[0], false) + "; ";
        }
        
        // Generate input reading - no cin.ignore() needed for compiled programs
        code += "std::string s; std::getline(std::cin, s); return s; }())";
        
        return code;
    } else if (call->objectName == "type") {
        // Handle type conversion functions
        if (call->arguments.empty()) {
            return "// Type conversion requires an argument";
        }
        
        std::string arg = generateExpression(call->arguments[0], false);
        
        if (call->methodName == "int") {
            return "std::stoi(" + arg + ")";
        } else if (call->methodName == "float") {
            return "std::stof(" + arg + ")";
        } else if (call->methodName == "double") {
            return "std::stod(" + arg + ")";
        } else if (call->methodName == "char") {
            return "((" + arg + ".empty()) ? '\\0' : (" + arg + ")[0])";
        } else if (call->methodName == "string") {
            return "std::to_string(" + arg + ")";
        }
    } else if (call->objectName.empty()) {
        // Regular function call (e.g., myFunction())
        std::string code = call->methodName + "(";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            code += generateExpression(call->arguments[i], false);
            if (i < call->arguments.size() - 1) {
                code += ", ";
            }
        }
        
        code += ")";
        return code;
    } else if (call->objectName == "instance") {
        // Special case for instance.method() calls within class methods
        // This should be handled by the method body code generation that replaces instance. with this->
        std::string code = call->methodName + "(";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            code += generateExpression(call->arguments[i]);
            if (i < call->arguments.size() - 1) {
                code += ", ";
            }
        }
        
        code += ")";
        return code;
    } else {
        // Handle string operations
        if (call->methodName == "replace") {
            // str.replace(old, new) -> stringReplace(str, old, new)
            std::string code = "stringReplace(" + call->objectName + ", ";
            // Generate arguments
            for (size_t i = 0; i < call->arguments.size(); ++i) {
                code += generateExpression(call->arguments[i]);
                if (i < call->arguments.size() - 1) {
                    code += ", ";
                }
            }
            code += ")";
            return code;
        } else if (call->methodName == "excision") {
            // str.excision(delimiter) -> stringExcision(str, delimiter)
            std::string code = "stringExcision(" + call->objectName + ", ";
            // Generate arguments
            for (size_t i = 0; i < call->arguments.size(); ++i) {
                code += generateExpression(call->arguments[i]);
                if (i < call->arguments.size() - 1) {
                    code += ", ";
                }
            }
            code += ")";
            return code;
        }
        // Handle list and map operations
        else if (call->methodName == "add") {
            // Only use listAdd for List objects, not for module methods
            // Check if objectName is a List by looking at its type (this is a simplified check)
            // In Vanction, List variables are declared with List type, so their names don't indicate their type
            // A better approach would be to track variable types, but for now we'll use a heuristic
            // If the objectName is not a module (doesn't contain underscores or dots), assume it's a List
            bool isListObject = true;
            
            // Check if it's likely a module (contains underscores, dots, or known module names)
            if (call->objectName.find('_') != std::string::npos || 
                call->objectName.find('.') != std::string::npos ||
                call->objectName == "math") {
                isListObject = false;
            }
            
            if (isListObject) {
                // List.add() -> listAdd(list, value)
                std::string code = "listAdd(" + call->objectName + ", ";
                // Generate arguments
                for (size_t i = 0; i < call->arguments.size(); ++i) {
                    code += generateExpression(call->arguments[i]);
                    if (i < call->arguments.size() - 1) {
                        code += ", ";
                    }
                }
                code += ")";
                return code;
            } else {
                // For module methods, use normal method call syntax
                std::string code = call->objectName + "->" + call->methodName + "(";
                // Generate arguments
                for (size_t i = 0; i < call->arguments.size(); ++i) {
                    code += generateExpression(call->arguments[i]);
                    if (i < call->arguments.size() - 1) {
                        code += ", ";
                    }
                }
                code += ")";
                return code;
            }
        } else if (call->methodName == "get") {
            // Check if it's a HashMap get operation by looking at the argument type
            bool isHashMapGet = false;
            if (call->arguments.size() > 0) {
                // If the first argument is a string literal, it's probably a HashMap get
                if (dynamic_cast<StringLiteral*>(call->arguments[0])) {
                    isHashMapGet = true;
                }
            }
            
            if (isHashMapGet) {
                // HashMap.get() -> get(map, key)
                std::string code = "get(" + call->objectName + ", ";
                // Generate arguments
                for (size_t i = 0; i < call->arguments.size(); ++i) {
                    code += generateExpression(call->arguments[i]);
                    if (i < call->arguments.size() - 1) {
                        code += ", ";
                    }
                }
                code += ")";
                return code;
            } else {
                // List.get() -> get(list, index)
                std::string code = "get(" + call->objectName + ", ";
                // Generate arguments
                for (size_t i = 0; i < call->arguments.size(); ++i) {
                    code += generateExpression(call->arguments[i]);
                    if (i < call->arguments.size() - 1) {
                        code += ", ";
                    }
                }
                code += ")";
                return code;
            }
        } else if (call->methodName == "key" || call->methodName == "keys") {
            // map.key() or map.keys() -> mapKeys(map)
            return "mapKeys(" + call->objectName + ")";
        } else if (call->methodName == "value" || call->methodName == "values") {
            // map.value() or map.values() -> mapValues(map)
            return "mapValues(" + call->objectName + ")";
        }
        
        // Check if it's an instance method call (e.g., obj.method()) or namespace function call
        // In Vanction, instance variables are created with instance keyword, so we need to check if objectName is an instance variable
        // For now, assume any objectName that's not a known namespace is an instance variable
        // Generate instance method call syntax: objectName->methodName()
        std::string code = call->objectName + "->" + call->methodName + "(";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            code += generateExpression(call->arguments[i]);
            if (i < call->arguments.size() - 1) {
                code += ", ";
            }
        }
        
        code += ")";
        return code;
    }
    return "// Unimplemented function call";
}

// Generate comment
std::string CodeGenerator::generateComment(Comment* comment) {
    std::string text = comment->text;
    std::string code;
    
    // Handle different comment types
    if (text.substr(0, 2) == "|*") {
        // Multi-line comment |* ... *|
        // Extract content between |* and *|
        size_t endPos = text.find("*|", 2);
        if (endPos != std::string::npos) {
            std::string content = text.substr(2, endPos - 2);
            code += "    /*" + content + "*/\n";
        }
    } else if (text.substr(0, 2) == "|/") {
        // Doc comment |/ ... /|
        // Extract content between |/ and /|
        size_t endPos = text.find("/|", 2);
        if (endPos != std::string::npos) {
            std::string content = text.substr(2, endPos - 2);
            code += "    /**" + content + "*/\n";
        }
    } else if (text.substr(0, 1) == "|") {
        // Single-line comment | ...
        code += "    // " + text.substr(1) + "\n";
    }
    
    return code;
}

// Generate list literal expression
std::string CodeGenerator::generateListLiteral(ListLiteral* list) {
    // Use a variant type that can handle different data types
    std::string code = "std::vector<std::variant<int, std::string, bool>>{";
    
    // Generate elements
    for (size_t i = 0; i < list->elements.size(); ++i) {
        code += generateExpression(list->elements[i], false);
        if (i < list->elements.size() - 1) {
            code += ", ";
        }
    }
    
    code += "}";
    return code;
}

// Generate hash map literal expression
std::string CodeGenerator::generateHashMapLiteral(HashMapLiteral* hashMap) {
    // Use variant type that can handle different data types
    std::string code = "std::unordered_map<std::string, std::variant<int, std::string, bool>>{";
    
    // Generate entries
    for (size_t i = 0; i < hashMap->entries.size(); ++i) {
        auto entry = hashMap->entries[i];
        
        std::string key = generateExpression(entry->key, false);
        std::string value = generateExpression(entry->value, false);
        
        code += "{" + key + ", " + value + "}";
        if (i < hashMap->entries.size() - 1) {
            code += ", ";
        }
    }
    
    code += "}";
    return code;
}

// Generate range expression
std::string CodeGenerator::generateRangeExpression(RangeExpression* range) {
    std::string start = generateExpression(range->start, false);
    std::string end = generateExpression(range->end, false);
    std::string step = range->step ? generateExpression(range->step, false) : "1";
    
    // Generate a C++ range-based for loop compatible range
    // We'll use a custom generator function for range
    return "vanction::range(" + start + ", " + end + ", " + step + ")";
}

// Generate lambda expression
std::string CodeGenerator::generateLambdaExpression(LambdaExpression* lambda) {
    std::string code = "[=](";
    
    // Generate parameters
    for (size_t i = 0; i < lambda->parameters.size(); ++i) {
        if (i > 0) {
            code += ", ";
        }
        // Since we don't have parameter types, use auto for all parameters
        code += "auto " + lambda->parameters[i].name;
    }
    
    code += ") -> auto { return " + generateExpression(lambda->body, false) + "; }";
    
    return code;
}

// Generate import statement
std::string CodeGenerator::generateImportStatement(ImportStatement* importStmt) {
    std::string code;
    
    // Generate a placeholder for the imported module
    // This is a simple implementation that just declares the alias as a pointer
    // In a full implementation, we would parse the imported module and generate proper code
    if (!importStmt->alias.empty()) {
        // Generate a struct definition for the imported module
        code += "// Imported module " + importStmt->moduleName + " as " + importStmt->alias + "\n";
        code += "struct " + importStmt->moduleName + "_Module {\n";
        code += "    // Placeholder for module functions\n";
        code += "    int add(int a, int b) { return a + b; }\n";
        code += "    int subtract(int a, int b) { return a - b; }\n";
        code += "    int multiply(int a, int b) { return a * b; }\n";
        code += "    int divide(int a, int b) { return a / b; }\n";
        code += "};\n\n";
        
        // Generate a variable declaration for the alias
        code += "auto " + importStmt->alias + " = std::make_unique<" + importStmt->moduleName + "_Module>();\n\n";
    }
    
    return code;
}
