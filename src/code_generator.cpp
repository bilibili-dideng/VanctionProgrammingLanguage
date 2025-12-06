#include "code_generator.h"
#include "../include/ast.h"
#include <iostream>

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

// Generate C++ code
std::string CodeGenerator::generate(Program* program) {
    std::string code;
    
    // Add header files
    code += "#include <iostream>\n#include <string>\n#include <memory>\n\n";
    
    // Generate all declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            code += generateFunctionDeclaration(func);
        } else if (auto ns = dynamic_cast<NamespaceDeclaration*>(decl)) {
            code += generateNamespaceDeclaration(ns);
        } else if (auto cls = dynamic_cast<ClassDeclaration*>(decl)) {
            code += generateClassDeclaration(cls);
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
        }
        
        // Generate function name
        code += returnType + " " + func->name + "(";
        
        // Generate parameters
        for (size_t i = 0; i < func->parameters.size(); ++i) {
            const auto& param = func->parameters[i];
            std::string paramType = param.type;
            if (paramType == "string") {
                paramType = "std::string";
            }
            code += paramType + " " + param.name;
            if (i < func->parameters.size() - 1) {
                code += ", ";
            }
        }
        
        code += ") {\n";
    }
    
    // Generate function body
    for (auto stmt : func->body) {
        if (auto comment = dynamic_cast<Comment*>(stmt)) {
            code += generateComment(comment);
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            code += generateExpressionStatement(exprStmt);
        } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
            code += generateVariableDeclaration(varDecl);
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
                code += " " + generateExpression(returnStmt->expression);
            }
            code += ";\n";
        } else if (auto nestedFunc = dynamic_cast<FunctionDeclaration*>(stmt)) {
            // Generate nested function declaration
            code += "    " + generateFunctionDeclaration(nestedFunc);
        } else {
            code += "    // Unimplemented statement type\n";
        }
    }
    
    // Generate function end
    code += "}\n\n";
    
    return code;
}

// Generate expression statement
std::string CodeGenerator::generateExpressionStatement(ExpressionStatement* stmt) {
    if (auto assignExpr = dynamic_cast<AssignmentExpression*>(stmt->expression)) {
        if (auto ident = dynamic_cast<Identifier*>(assignExpr->left)) {
            // Handle assignment to undeclared variable by declaring it as auto
            return "    auto " + generateAssignmentExpression(assignExpr) + ";\n";
        }
    }
    return "    " + generateExpression(stmt->expression) + ";\n";
}

// Generate variable declaration
std::string CodeGenerator::generateVariableDeclaration(VariableDeclaration* varDecl) {
    std::string code = "    ";
    
    if (varDecl->isDefine) {
        // Define statement: use std::string with empty value
        code += "std::string " + varDecl->name + ";\n";
    } else if (varDecl->isAuto) {
        // Auto type inference
        code += "auto " + varDecl->name;
        if (varDecl->initializer) {
            code += " = " + generateExpression(varDecl->initializer);
        }
        code += ";\n";
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
std::string CodeGenerator::generateExpression(Expression* expr) {
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
        return generateBinaryExpression(binaryExpr);
    } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(expr)) {
        return generateAssignmentExpression(assignExpr);
    } else if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
        return generateFunctionCall(funcCall);
    } else if (auto instanceCreate = dynamic_cast<InstanceCreationExpression*>(expr)) {
        return generateInstanceCreationExpression(instanceCreate);
    } else if (auto instanceAccess = dynamic_cast<InstanceAccessExpression*>(expr)) {
        return generateInstanceAccessExpression(instanceAccess);
    }
    return "// Unimplemented expression";
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
    return '"' + literal->value + '"';
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
std::string CodeGenerator::generateBinaryExpression(BinaryExpression* expr) {
    std::string left = generateExpression(expr->left);
    std::string right = generateExpression(expr->right);
    std::string op = expr->op;
    
    // Handle different operators
    if (op == "+") {
        return left + " + " + right;
    } else if (op == "-") {
        return left + " - " + right;
    } else if (op == "*") {
        return left + " * " + right;
    } else if (op == "/") {
        return left + " / " + right;
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
    }
    
    return left + " " + op + " " + right;
}

// Generate if statement
std::string CodeGenerator::generateIfStatement(IfStatement* stmt) {
    std::string code = "    if (" + generateExpression(stmt->condition) + ") {\n";
    
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
        code += " else if (" + generateExpression(elseIf->condition) + ") {\n";
        
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
        code += generateVariableDeclaration(varDecl).substr(4); // Remove indentation
        code.pop_back(); // Remove newline
    } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt->initialization)) {
        code += generateExpression(exprStmt->expression);
    }
    
    code += "; " + generateExpression(stmt->condition) + "; " + generateExpression(stmt->increment) + ") {\n";
    
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
    std::string code = "    for (char " + stmt->variableName + " : " + generateExpression(stmt->collection) + ") {\n";
    
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

// Generate while loop statement
std::string CodeGenerator::generateWhileLoopStatement(WhileLoopStatement* stmt) {
    std::string code = "    while (" + generateExpression(stmt->condition) + ") {\n";
    
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
    
    code += "    } while (" + generateExpression(stmt->condition) + ");\n";
    return code;
}

// Generate case statement
std::string CodeGenerator::generateCaseStatement(CaseStatement* stmt) {
    std::string code = "    case " + generateExpression(stmt->value) + ": {\n";
    
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
    std::string code = "    switch (" + generateExpression(stmt->expression) + ") {\n";
    
    // Generate case statements
    for (auto caseStmt : stmt->cases) {
        code += generateCaseStatement(caseStmt);
    }
    
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
    
    // Add member variables (name, age, and id are common in examples)
    code += "    std::string name;\n";
    code += "    int age;\n";
    code += "    int id;\n\n";
    
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
            } else if (param.name == "age" || param.name == "id") {
                paramType = "int";
            } else {
                paramType = "auto";
            }
            code += paramType + " " + param.name;
            if (i < initMethod->parameters.size() - 1) {
                code += ", ";
            }
        }
        code += ") {\n";
        
        // Generate constructor body
        for (auto stmt : initMethod->body) {
            if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
                std::string stmtCode = generateExpressionStatement(exprStmt).substr(4);
                // Replace instance. and instance-> with this-> in constructor body
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
                paramType = "auto";
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
            paramType = "auto";
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
        code += generateExpression(expr->arguments[i]);
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
    return generateExpression(expr->instance) + "->" + expr->memberName;
}

// Generate assignment expression
std::string CodeGenerator::generateAssignmentExpression(AssignmentExpression* expr) {
    return generateExpression(expr->left) + " = " + generateExpression(expr->right);
}

// Generate function call
std::string CodeGenerator::generateFunctionCall(FunctionCall* call) {
    if (call->objectName == "System" && call->methodName == "print") {
        // Generate std::cout << ... << std::endl
        std::string code = "std::cout";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            std::string argExpr = generateExpression(call->arguments[i]);
            
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
            code += "std::cout << " + generateExpression(call->arguments[0]) + "; ";
        }
        
        // Generate input reading
        code += "std::string s; std::getline(std::cin, s); return s; }())";
        
        return code;
    } else if (call->objectName == "type") {
        // Handle type conversion functions
        if (call->arguments.empty()) {
            return "// Type conversion requires an argument";
        }
        
        std::string arg = generateExpression(call->arguments[0]);
        
        if (call->methodName == "int") {
            return "std::stoi(" + arg + ")";
        } else if (call->methodName == "float") {
            return "std::stof(" + arg + ")";
        } else if (call->methodName == "double") {
            return "std::stod(" + arg + ")";
        } else if (call->methodName == "char") {
            return "static_cast<char>(std::stoi(" + arg + "))";
        } else if (call->methodName == "string") {
            return "std::to_string(" + arg + ")";
        }
    } else if (call->objectName.empty()) {
        // Regular function call (e.g., myFunction())
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
    return "    // " + comment->text.substr(1) + "\n"; // Remove | symbol
}
