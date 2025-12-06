#ifndef VANCTION_CODE_GENERATOR_H
#define VANCTION_CODE_GENERATOR_H

#include "../include/ast.h"
#include <string>

// Code generator class
class CodeGenerator {
public:
    // Generate C++ code
    std::string generate(Program* program);
    
private:
    // Generate function declaration
    std::string generateFunctionDeclaration(FunctionDeclaration* func);
    
    // Generate expression statement
    std::string generateExpressionStatement(ExpressionStatement* stmt);
    
    // Generate expression
    std::string generateExpression(Expression* expr);
    
    // Generate variable declaration
    std::string generateVariableDeclaration(VariableDeclaration* varDecl);
    
    // Generate identifier
    std::string generateIdentifier(Identifier* ident);
    
    // Generate integer literal
    std::string generateIntegerLiteral(IntegerLiteral* literal);
    
    // Generate char literal
    std::string generateCharLiteral(CharLiteral* literal);
    
    // Generate string literal
    std::string generateStringLiteral(StringLiteral* literal);
    
    // Generate boolean literal
    std::string generateBooleanLiteral(BooleanLiteral* literal);
    
    // Generate float literal
    std::string generateFloatLiteral(FloatLiteral* literal);
    
    // Generate double literal
    std::string generateDoubleLiteral(DoubleLiteral* literal);
    
    // Generate binary expression
    std::string generateBinaryExpression(BinaryExpression* expr);
    
    // Generate assignment expression
    std::string generateAssignmentExpression(AssignmentExpression* expr);
    
    // Generate function call
    std::string generateFunctionCall(FunctionCall* call);
    
    // Generate namespace declaration
    std::string generateNamespaceDeclaration(NamespaceDeclaration* ns);
    
    // Generate namespace access
    std::string generateNamespaceAccess(NamespaceAccess* access);
    
    // Generate comment
    std::string generateComment(Comment* comment);
    
    // Generate if statement
    std::string generateIfStatement(IfStatement* stmt);
    
    // Generate for loop statement
    std::string generateForLoopStatement(ForLoopStatement* stmt);
    
    // Generate for-in loop statement
    std::string generateForInLoopStatement(ForInLoopStatement* stmt);
    
    // Generate while loop statement
    std::string generateWhileLoopStatement(WhileLoopStatement* stmt);
    
    // Generate do-while loop statement
    std::string generateDoWhileLoopStatement(DoWhileLoopStatement* stmt);
    
    // Generate case statement
    std::string generateCaseStatement(CaseStatement* stmt);
    
    // Generate switch statement
    std::string generateSwitchStatement(SwitchStatement* stmt);
    
    // Generate class declaration
    std::string generateClassDeclaration(ClassDeclaration* cls);
    
    // Generate class method declaration
    std::string generateClassMethodDeclaration(ClassMethodDeclaration* method);
    
    // Generate instance method declaration
    std::string generateInstanceMethodDeclaration(InstanceMethodDeclaration* method);
    
    // Generate instance creation expression
    std::string generateInstanceCreationExpression(InstanceCreationExpression* expr);
    
    // Generate instance access expression
    std::string generateInstanceAccessExpression(InstanceAccessExpression* expr);
};

#endif // VANCTION_CODE_GENERATOR_H
