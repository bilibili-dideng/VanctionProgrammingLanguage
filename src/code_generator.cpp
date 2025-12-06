#include "code_generator.h"
#include "../include/ast.h"
#include <iostream>

// Generate C++ code
std::string CodeGenerator::generate(Program* program) {
    std::string code;
    
    // Add header files
    code += "#include <iostream>\n\n";
    
    // Generate all declarations
    for (auto decl : program->declarations) {
        if (auto func = dynamic_cast<FunctionDeclaration*>(decl)) {
            code += generateFunctionDeclaration(func);
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
        code += "void " + func->name + "() {\n";
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
        } else {
            code += "    // Unimplemented statement type\n";
        }
    }
    
    // Generate function end
    if (func->name == "main") {
        code += "    return 0;\n";
    }
    code += "}\n\n";
    
    return code;
}

// Generate expression statement
std::string CodeGenerator::generateExpressionStatement(ExpressionStatement* stmt) {
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

// Generate expression
std::string CodeGenerator::generateExpression(Expression* expr) {
    if (auto ident = dynamic_cast<Identifier*>(expr)) {
        return generateIdentifier(ident);
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

// Generate assignment expression
std::string CodeGenerator::generateAssignmentExpression(AssignmentExpression* expr) {
    return generateExpression(expr->left) + " = " + generateExpression(expr->right);
}

// Generate function call
std::string CodeGenerator::generateFunctionCall(FunctionCall* call) {
    if (call->objectName == "System" && call->methodName == "print") {
        // Generate std::cout << ... << std::endl
        std::string code = "std::cout << ";
        
        // Generate arguments
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            code += generateExpression(call->arguments[i]);
            if (i < call->arguments.size() - 1) {
                code += " << ";
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
    }
    return "// Unimplemented function call";
}

// Generate comment
std::string CodeGenerator::generateComment(Comment* comment) {
    return "    // " + comment->text.substr(1) + "\n"; // Remove | symbol
}
