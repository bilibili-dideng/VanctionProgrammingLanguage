#include "parser.h"
#include <iostream>
#include <cstdlib>

// Constructor
Parser::Parser(Lexer& lexer) {
    this->lexer = &lexer;
    this->currentToken = lexer.getNextToken();
}

// Parse program (validate syntax)
bool Parser::parseProgram() {
    // Program should start with main function
    if (parseFunction()) {
        // Check if it's main function
        if (functionName == "main") {
            return true;
        } else {
            std::cerr << "Error: Program must start with main function" << std::endl;
            return false;
        }
    }
    return false;
}

// Parse program and generate AST
Program* Parser::parseProgramAST() {
    auto program = new Program();
    
    // Parse function definition
    auto func = parseFunctionAST();
    if (func) {
        program->declarations.push_back(func);
    }
    
    return program;
}

// Consume current token
void Parser::consume(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        currentToken = lexer->getNextToken();
    } else {
        std::cerr << "Syntax error: expected " << tokenTypeToString(expectedType) << ", but got " << tokenTypeToString(currentToken.type) << " at line " << currentToken.line << " column " << currentToken.column << std::endl;
        exit(1);
    }
}

// Parse function definition
bool Parser::parseFunction() {
    // Check if it's func keyword
    if (currentToken.type != KEYWORD || currentToken.value != "func") {
        std::cerr << "Syntax error: Function definition must start with 'func' keyword" << std::endl;
        return false;
    }
    consume(KEYWORD);
    
    // Parse function name
    if (currentToken.type != IDENTIFIER) {
        std::cerr << "Syntax error: Function name must be an identifier" << std::endl;
        return false;
    }
    functionName = currentToken.value;
    consume(IDENTIFIER);
    
    // Parse left parenthesis
    consume(LPAREN);
    
    // Parse right parenthesis
    consume(RPAREN);
    
    // Parse left brace
    consume(LBRACE);
    
    // Parse function body (skip all content until right brace)
    parseFunctionBody();
    
    // Parse right brace
    consume(RBRACE);
    
    return true;
}

// Parse function definition and generate AST
FunctionDeclaration* Parser::parseFunctionAST() {
    // Check if it's func keyword
    if (currentToken.type != KEYWORD || currentToken.value != "func") {
        std::cerr << "Syntax error: Function definition must start with 'func' keyword" << std::endl;
        return nullptr;
    }
    consume(KEYWORD);
    
    // Parse function name
    if (currentToken.type != IDENTIFIER) {
        std::cerr << "Syntax error: Function name must be an identifier" << std::endl;
        return nullptr;
    }
    std::string funcName = currentToken.value;
    consume(IDENTIFIER);
    
    // Parse left parenthesis
    consume(LPAREN);
    
    // Parse right parenthesis
    consume(RPAREN);
    
    // Parse left brace
    consume(LBRACE);
    
    // Parse function body
    auto body = parseFunctionBodyAST();
    
    // Parse right brace
    consume(RBRACE);
    
    // Create function declaration node
    auto func = new FunctionDeclaration(funcName);
    func->body = std::move(body);
    
    return func;
}

// Parse function body
void Parser::parseFunctionBody() {
    while (currentToken.type != RBRACE && currentToken.type != EOF_TOKEN) {
        // Skip all content, including comments
        currentToken = lexer->getNextToken();
    }
}

// Parse function body and generate AST
std::vector<ASTNode*> Parser::parseFunctionBodyAST() {
    std::vector<ASTNode*> body;
    
    while (currentToken.type != RBRACE && currentToken.type != EOF_TOKEN) {
        if (currentToken.type == COMMENT) {
            // Create comment node
            auto comment = new Comment(currentToken.value);
            body.push_back(comment);
            
            // Move to next token
            currentToken = lexer->getNextToken();
        } else {
            // Parse statement
            auto stmt = parseStatement();
            if (stmt) {
                body.push_back(stmt);
            }
        }
    }
    
    return body;
}

// Parse statement
Statement* Parser::parseStatement() {
    // Check for variable declaration
    if (currentToken.type == KEYWORD && 
        (currentToken.value == "int" || currentToken.value == "char" || currentToken.value == "string" || 
         currentToken.value == "bool" || currentToken.value == "float" || currentToken.value == "double" ||
         currentToken.value == "auto" || currentToken.value == "define")) {
        return parseVariableDeclaration();
    }
    
    // Parse expression
    auto expr = parseExpression();
    if (!expr) {
        return nullptr;
    }
    
    // Expect semicolon
    consume(SEMICOLON);
    
    // Create expression statement
    return new ExpressionStatement(expr);
}

// Parse variable declaration
Statement* Parser::parseVariableDeclaration() {
    bool isDefine = false;
    bool isAuto = false;
    std::string type;
    
    // Check if it's a define statement
    if (currentToken.value == "define") {
        isDefine = true;
        consume(KEYWORD);
    } else {
        // Check if it's auto type
        if (currentToken.value == "auto") {
            isAuto = true;
            consume(KEYWORD);
        } else {
            // Parse explicit type
            type = currentToken.value;
            consume(KEYWORD);
        }
    }
    
    // Parse variable name
    std::string name = currentToken.value;
    consume(IDENTIFIER);
    
    Expression* initializer = nullptr;
    
    // Check for assignment
    if (currentToken.type == ASSIGN) {
        consume(ASSIGN);
        initializer = parseExpression();
    }
    
    // Expect semicolon
    consume(SEMICOLON);
    
    // Create variable declaration node
    return new VariableDeclaration(type, name, initializer, isAuto, isDefine);
}

// Parse expression
Expression* Parser::parseExpression() {
    // Parse assignment expression
    return parseAssignmentExpression();
}

// Parse assignment expression
Expression* Parser::parseAssignmentExpression() {
    auto left = parseBinaryExpression();
    
    if (currentToken.type == ASSIGN) {
        consume(ASSIGN);
        auto right = parseAssignmentExpression();
        
        // Check if left is an identifier
        if (auto ident = dynamic_cast<Identifier*>(left)) {
            return new AssignmentExpression(ident, right);
        } else {
            std::cerr << "Syntax error: left side of assignment must be an identifier" << std::endl;
            exit(1);
        }
    }
    
    return left;
}

// Parse multiplication and division expressions
Expression* Parser::parseMultiplicativeExpression() {
    auto left = parsePrimaryExpression();
    
    while (true) {
        if (currentToken.type == MULTIPLY) {
            consume(MULTIPLY);
            auto right = parsePrimaryExpression();
            left = new BinaryExpression(left, "*", right);
        } else if (currentToken.type == DIVIDE) {
            consume(DIVIDE);
            auto right = parsePrimaryExpression();
            left = new BinaryExpression(left, "/", right);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse addition and subtraction expressions
Expression* Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    
    while (true) {
        if (currentToken.type == PLUS) {
            consume(PLUS);
            auto right = parseMultiplicativeExpression();
            left = new BinaryExpression(left, "+", right);
        } else if (currentToken.type == MINUS) {
            consume(MINUS);
            auto right = parseMultiplicativeExpression();
            left = new BinaryExpression(left, "-", right);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse bit shift expressions
Expression* Parser::parseBitShiftExpression() {
    auto left = parseAdditiveExpression();
    
    while (true) {
        if (currentToken.type == LSHIFT) {
            consume(LSHIFT);
            auto right = parseAdditiveExpression();
            left = new BinaryExpression(left, "<<", right);
        } else if (currentToken.type == RSHIFT) {
            consume(RSHIFT);
            auto right = parseAdditiveExpression();
            left = new BinaryExpression(left, ">>", right);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse logical expressions (AND, OR, XOR)
Expression* Parser::parseLogicalExpression() {
    auto left = parseBitShiftExpression();
    
    while (currentToken.type == KEYWORD) {
        if (currentToken.value == "AND") {
            consume(KEYWORD);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "AND", right);
        } else if (currentToken.value == "OR") {
            consume(KEYWORD);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "OR", right);
        } else if (currentToken.value == "XOR") {
            consume(KEYWORD);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "XOR", right);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse binary expression (now delegates to logical expression)
Expression* Parser::parseBinaryExpression() {
    return parseLogicalExpression();
}

// Parse primary expression
Expression* Parser::parsePrimaryExpression() {
    // Check for identifier
    if (currentToken.type == IDENTIFIER) {
        std::string name = currentToken.value;
        currentToken = lexer->getNextToken();
        
        // Check if it's a function call (e.g., System.print)
        if (currentToken.type == DOT) {
            // It's a method call, parse it as function call
            currentToken = lexer->getNextToken(); // consume dot
            
            // Set up for function call parsing
            // We need to reconstruct the function call parsing since we already consumed the object name
            std::string methodName = currentToken.value;
            currentToken = lexer->getNextToken();
            
            // Expect left parenthesis
            consume(LPAREN);
            
            // Create function call node
            auto call = new FunctionCall(name, methodName);
            
            // Parse arguments
            if (currentToken.type != RPAREN) {
                auto arg = parseExpression();
                if (arg) {
                    call->arguments.push_back(arg);
                }
            }
            
            // Expect right parenthesis
            consume(RPAREN);
            
            return call;
        } else {
            // It's a simple identifier
            return new Identifier(name);
        }
    }
    
    // Check for string literal
    if (currentToken.type == STRING_LITERAL) {
        return parseStringLiteral();
    }
    
    // Check for integer literal
    if (currentToken.type == INTEGER_LITERAL) {
        return parseIntegerLiteral();
    }
    
    // Check for float literal
    if (currentToken.type == FLOAT_LITERAL) {
        return parseFloatLiteral();
    }
    
    // Check for double literal
    if (currentToken.type == DOUBLE_LITERAL) {
        return parseDoubleLiteral();
    }
    
    // Check for char literal
    if (currentToken.type == CHAR_LITERAL) {
        return parseCharLiteral();
    }
    
    // Check for boolean literal or logical operator
    if (currentToken.type == KEYWORD) {
        if (currentToken.value == "true" || currentToken.value == "false") {
            return parseBooleanLiteral();
        } else if (currentToken.value == "AND" || currentToken.value == "OR" || currentToken.value == "XOR") {
            // Logical operators are handled in binary expression parsing
            std::cerr << "Syntax error: Unexpected logical operator at line " << currentToken.line << " column " << currentToken.column << std::endl;
            exit(1);
        }
    }
    
    // Unexpected token
    std::cerr << "Syntax error: Unexpected token at line " << currentToken.line << " column " << currentToken.column << std::endl;
    exit(1);
}

// Parse string literal
Expression* Parser::parseStringLiteral() {
    std::string value = currentToken.value;
    // Remove quotes
    if (value.size() >= 2) {
        value = value.substr(1, value.size() - 2);
    }
    currentToken = lexer->getNextToken();
    return new StringLiteral(value);
}

// Parse integer literal
Expression* Parser::parseIntegerLiteral() {
    int value = std::stoi(currentToken.value);
    currentToken = lexer->getNextToken();
    return new IntegerLiteral(value);
}

// Parse char literal
Expression* Parser::parseCharLiteral() {
    std::string value = currentToken.value;
    char charValue = '\0';
    // Remove quotes and get char
    if (value.size() >= 2) {
        charValue = value[1];
    }
    currentToken = lexer->getNextToken();
    return new CharLiteral(charValue);
}

// Parse float literal
Expression* Parser::parseFloatLiteral() {
    float value = std::stof(currentToken.value);
    currentToken = lexer->getNextToken();
    return new FloatLiteral(value);
}

// Parse double literal
Expression* Parser::parseDoubleLiteral() {
    double value = std::stod(currentToken.value);
    currentToken = lexer->getNextToken();
    return new DoubleLiteral(value);
}

// Parse boolean literal
Expression* Parser::parseBooleanLiteral() {
    bool value = (currentToken.value == "true");
    currentToken = lexer->getNextToken();
    return new BooleanLiteral(value);
}

// Parse function call
Expression* Parser::parseFunctionCall() {
    // Parse object name (e.g., System)
    std::string objectName = currentToken.value;
    consume(IDENTIFIER);
    
    // Expect dot
    consume(DOT);
    
    // Parse method name (e.g., print or input)
    std::string methodName = currentToken.value;
    consume(IDENTIFIER);
    
    // Expect left parenthesis
    consume(LPAREN);
    
    // Create function call node
    auto call = new FunctionCall(objectName, methodName);
    
    // Parse arguments
    if (currentToken.type != RPAREN) {
        auto arg = parseExpression();
        if (arg) {
            call->arguments.push_back(arg);
        }
    }
    
    // Expect right parenthesis
    consume(RPAREN);
    
    return call;
}

// Helper function: convert token type to string
std::string Parser::tokenTypeToString(TokenType type) {
    switch (type) {
        case KEYWORD: return "keyword";
        case IDENTIFIER: return "identifier";
        case STRING_LITERAL: return "string literal";
        case CHAR_LITERAL: return "char literal";
        case INTEGER_LITERAL: return "integer literal";
        case FLOAT_LITERAL: return "float literal";
        case DOUBLE_LITERAL: return "double literal";
        case DOT: return "dot";
        case SEMICOLON: return "semicolon";
        case LPAREN: return "left parenthesis";
        case RPAREN: return "right parenthesis";
        case LBRACE: return "left brace";
        case RBRACE: return "right brace";
        case ASSIGN: return "assignment operator";
        case PLUS: return "plus operator";
        case MINUS: return "minus operator";
        case MULTIPLY: return "multiply operator";
        case DIVIDE: return "divide operator";
        case LSHIFT: return "left shift operator";
        case RSHIFT: return "right shift operator";
        case AND: return "and operator";
        case OR: return "or operator";
        case XOR: return "xor operator";
        case COMMENT: return "comment";
        case EOF_TOKEN: return "end of file";
        default: return "unknown type";
    }
}
