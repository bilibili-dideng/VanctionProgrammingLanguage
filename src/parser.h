#ifndef VANCTION_PARSER_H
#define VANCTION_PARSER_H

#include "lexer.h"
#include "../include/ast.h"
#include <string>

// Parser class
class Parser {
public:
    // Constructor
    Parser(Lexer& lexer);
    
    // Parse program (validate syntax)
    bool parseProgram();
    
    // Parse program and generate AST
    Program* parseProgramAST();
    
private:
    Lexer* lexer;
    Token currentToken;
    std::string functionName;
    
    // Consume current token
    void consume(TokenType expectedType);
    
    // Parse function definition
    bool parseFunction();
    
    // Parse function definition and generate AST
    FunctionDeclaration* parseFunctionAST();
    
    // Parse namespace declaration and generate AST
    NamespaceDeclaration* parseNamespaceDeclarationAST();
    
    // Parse class declaration and generate AST
    ClassDeclaration* parseClassDeclarationAST();
    
    // Parse function body
    void parseFunctionBody();
    
    // Parse function body and generate AST
    std::vector<ASTNode*> parseFunctionBodyAST();
    
    // Helper function: convert token type to string
    std::string tokenTypeToString(TokenType type);
    
    // Parse statement
    Statement* parseStatement();
    
    // Parse expression
    Expression* parseExpression();
    
    // Parse variable declaration
    Statement* parseVariableDeclaration();
    
    // Parse primary expression
    Expression* parsePrimaryExpression();
    
    // Parse string literal
    Expression* parseStringLiteral();
    
    // Parse integer literal
    Expression* parseIntegerLiteral();
    
    // Parse float literal
    Expression* parseFloatLiteral();
    
    // Parse double literal
    Expression* parseDoubleLiteral();
    
    // Parse char literal
    Expression* parseCharLiteral();
    
    // Parse boolean literal
    Expression* parseBooleanLiteral();
    
    // Parse assignment expression
    Expression* parseAssignmentExpression();
    
    // Parse binary expression
    Expression* parseBinaryExpression();
    
    // Parse comparison expression
    Expression* parseComparisonExpression();
    
    // Parse multiplicative expression
    Expression* parseMultiplicativeExpression();
    
    // Parse additive expression
    Expression* parseAdditiveExpression();
    
    // Parse bit shift expression
    Expression* parseBitShiftExpression();
    
    // Parse logical expression
    Expression* parseLogicalExpression();
    
    // Parse function call
    Expression* parseFunctionCall();
    
    // Parse block of code
    std::vector<ASTNode*> parseBlock();
    
    // Parse if statement
    Statement* parseIfStatement();
    
    // Parse for loop statement
    Statement* parseForLoopStatement();
    
    // Parse for-in loop statement
    Statement* parseForInLoopStatement();
    
    // Parse while loop statement
    Statement* parseWhileLoopStatement();
    
    // Parse do-while loop statement
    Statement* parseDoWhileLoopStatement();
    
    // Parse case statement for switch
    CaseStatement* parseCaseStatement();
    
    // Parse switch statement
    Statement* parseSwitchStatement();
};

#endif // VANCTION_PARSER_H
