#include "parser.h"
#include <iostream>
#include <cstdlib>
#include <stdexcept>

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
        std::string errorMessage = "expected " + tokenTypeToString(expectedType) + ", but got " + tokenTypeToString(currentToken.type) + " at line " + std::to_string(currentToken.line) + " column " + std::to_string(currentToken.column);
        throw std::runtime_error(errorMessage);
    }
}

// Parse function definition
bool Parser::parseFunction() {
    // Check if it's func keyword
    if (currentToken.type != KEYWORD || currentToken.value != "func") {
        throw std::runtime_error("Function definition must start with 'func' keyword");
    }
    consume(KEYWORD);
    
    // Parse function name
    if (currentToken.type != IDENTIFIER) {
        throw std::runtime_error("Function name must be an identifier");
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
        throw std::runtime_error("Syntax error: Function definition must start with 'func' keyword");
    }
    consume(KEYWORD);
    
    // Parse function name
    if (currentToken.type != IDENTIFIER) {
        throw std::runtime_error("Syntax error: Function name must be an identifier");
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

// Parse block of code
std::vector<ASTNode*> Parser::parseBlock() {
    consume(LBRACE);
    auto block = parseFunctionBodyAST();
    consume(RBRACE);
    return block;
}

// Parse if statement
Statement* Parser::parseIfStatement() {
    // Consume 'if' keyword
    consume(KEYWORD);
    
    // Consume '('
    consume(LPAREN);
    
    // Parse condition
    auto condition = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Parse if body
    auto ifBody = parseBlock();
    
    // Create if statement node
    auto ifStmt = new IfStatement(condition, ifBody);
    
    // Parse else-if clauses and else clause
    while (true) {
        // Check if it's 'else' or 'else-if'
        if (currentToken.type == KEYWORD && (currentToken.value == "else" || currentToken.value == "else-if")) {
            bool isElseIf = (currentToken.value == "else-if");
            
            // Consume 'else' or 'else-if' keyword
            consume(KEYWORD);
            
            if (isElseIf) {
                // It's an else-if clause
                // Consume '('
                consume(LPAREN);
                
                // Parse else-if condition
                auto elseIfCondition = parseExpression();
                
                // Consume ')'
                consume(RPAREN);
                
                // Parse else-if body
                auto elseIfBody = parseBlock();
                
                // Create else-if statement
                auto elseIfStmt = new IfStatement(elseIfCondition, elseIfBody);
                ifStmt->elseIfs.push_back(elseIfStmt);
            } else {
                // It's a simple else clause
                // Parse else body
                auto elseBody = parseBlock();
                ifStmt->elseBody = elseBody;
                break;
            }
        } else {
            break;
        }
    }
    
    return ifStmt;
}

// Parse for loop statement
Statement* Parser::parseForLoopStatement() {
    // Consume '('
    consume(LPAREN);
    
    // Parse initialization (allow variable declaration or assignment)
    Statement* initialization = nullptr;
    
    // Check if it's an assignment expression
    if (currentToken.type == IDENTIFIER) {
        // Look ahead to see if it's an assignment
        Token nextToken = lexer->getNextToken();
        if (nextToken.type == ASSIGN) {
            // It's an assignment expression, parse it as expression statement
            currentToken = nextToken;
            auto expr = parseExpression();
            initialization = new ExpressionStatement(expr);
        } else {
            // It's a variable declaration, parse it as statement
            // This approach doesn't require private member access
            currentToken = nextToken;
            // Reconstruct the token stream by parsing the statement
            initialization = parseStatement();
        }
    } else {
        // It's a variable declaration, parse it as statement
        initialization = parseStatement();
    }
    
    // Expect semicolon after initialization
    consume(SEMICOLON);
    
    // Parse condition
    auto condition = parseExpression();
    
    // Expect semicolon after condition
    consume(SEMICOLON);
    
    // Parse increment
    auto increment = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Parse loop body
    auto body = parseBlock();
    
    // Create for loop statement node
    return new ForLoopStatement(initialization, condition, increment, body);
}

// Parse for-in loop statement
Statement* Parser::parseForInLoopStatement() {
    // Consume 'for' keyword
    consume(KEYWORD);
    
    // Consume '('
    consume(LPAREN);
    
    // Parse variable name
    std::string varName = currentToken.value;
    consume(IDENTIFIER);
    
    // Consume 'in' keyword
    consume(KEYWORD);
    
    // Parse collection expression
    auto collection = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Parse loop body
    auto body = parseBlock();
    
    // Create for-in loop statement node
    return new ForInLoopStatement(varName, collection, body);
}

// Parse while loop statement
Statement* Parser::parseWhileLoopStatement() {
    // Consume 'while' keyword
    consume(KEYWORD);
    
    // Consume '('
    consume(LPAREN);
    
    // Parse condition
    auto condition = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Parse loop body
    auto body = parseBlock();
    
    // Create while loop statement node
    return new WhileLoopStatement(condition, body);
}

// Parse do-while loop statement
Statement* Parser::parseDoWhileLoopStatement() {
    // Consume 'do' keyword
    consume(KEYWORD);
    
    // Parse loop body
    auto body = parseBlock();
    
    // Consume 'while' keyword
    consume(KEYWORD);
    
    // Consume '('
    consume(LPAREN);
    
    // Parse condition
    auto condition = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Create do-while loop statement node
    return new DoWhileLoopStatement(body, condition);
}

// Parse case statement for switch
CaseStatement* Parser::parseCaseStatement() {
    // Consume 'case' keyword
    consume(KEYWORD);
    
    // Parse case value
    auto value = parseExpression();
    
    // Parse case body
    auto body = parseBlock();
    
    // Create case statement node
    return new CaseStatement(value, body);
}

// Parse switch statement
Statement* Parser::parseSwitchStatement() {
    // Consume 'switch' keyword
    consume(KEYWORD);
    
    // Consume '('
    consume(LPAREN);
    
    // Parse expression
    auto expression = parseExpression();
    
    // Consume ')'
    consume(RPAREN);
    
    // Consume '{'
    consume(LBRACE);
    
    // Parse case statements
    std::vector<CaseStatement*> cases;
    while (currentToken.type == KEYWORD && currentToken.value == "case") {
        auto caseStmt = parseCaseStatement();
        cases.push_back(caseStmt);
    }
    
    // Consume '}'
    consume(RBRACE);
    
    // Create switch statement node
    return new SwitchStatement(expression, cases);
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
    
    // Check for if statement
    if (currentToken.type == KEYWORD && currentToken.value == "if") {
        return parseIfStatement();
    }
    
    // Check for for loop statement
    if (currentToken.type == KEYWORD && currentToken.value == "for") {
        // Parse the 'for' keyword
        consume(KEYWORD);
        
        // Consume '('
        consume(LPAREN);
        
        // We'll use a simpler approach to distinguish between for-in and traditional for loops
        // Check if the next token after identifier is 'in'
        // This approach doesn't require private member access
        
        // Parse the first token inside for loop
        Token loopToken = currentToken;
        std::string varName;
        
        // Check if it's a type specifier followed by identifier (for-in loop with type annotation)
        if (loopToken.type == KEYWORD && 
            (loopToken.value == "int" || loopToken.value == "char" || loopToken.value == "string" || 
             loopToken.value == "bool" || loopToken.value == "float" || loopToken.value == "double")) {
            // It's a type specifier, consume it and parse the identifier
            consume(KEYWORD);
            
            if (currentToken.type == IDENTIFIER) {
                varName = currentToken.value;
                consume(IDENTIFIER);
                
                // Check if next token is 'in'
                if (currentToken.type == KEYWORD && currentToken.value == "in") {
                    // It's a for-in loop
                    // Consume 'in' keyword
                    consume(KEYWORD);
                    
                    // Parse collection expression
                    auto collection = parseExpression();
                    
                    // Consume ')'
                    consume(RPAREN);
                    
                    // Parse loop body
                    auto body = parseBlock();
                    
                    // Create for-in loop statement node
                    return new ForInLoopStatement(varName, collection, body);
                }
            }
        } 
        // If it's an identifier, check if next token is 'in' keyword
        else if (loopToken.type == IDENTIFIER) {
            // Consume the identifier (we'll use it later if it's for-in)
            varName = loopToken.value;
            consume(IDENTIFIER);
            
            // Check if next token is 'in'
            if (currentToken.type == KEYWORD && currentToken.value == "in") {
                // It's a for-in loop
                // Consume 'in' keyword
                consume(KEYWORD);
                
                // Parse collection expression
                auto collection = parseExpression();
                
                // Consume ')'
                consume(RPAREN);
                
                // Parse loop body
                auto body = parseBlock();
                
                // Create for-in loop statement node
                return new ForInLoopStatement(varName, collection, body);
            } else {
                // It's a traditional for loop with variable assignment
                // We need to parse i = 1 as initialization
                // Reconstruct the identifier by adding it back to the source
                // This is a safer approach that allows us to handle assignment expressions
                
                // Create an assignment expression manually
                auto ident = new Identifier(varName);
                
                // Check if next token is assignment operator
                if (currentToken.type == ASSIGN) {
                    consume(ASSIGN);
                    auto right = parseExpression();
                    auto assignment = new AssignmentExpression(ident, right);
                    Statement* initialization = new ExpressionStatement(assignment);
                    
                    // Expect semicolon after initialization
                    consume(SEMICOLON);
                    
                    // Parse condition
                    auto condition = parseExpression();
                    
                    // Expect semicolon after condition
                    consume(SEMICOLON);
                    
                    // Parse increment
                    auto increment = parseExpression();
                    
                    // Consume ')'
                    consume(RPAREN);
                    
                    // Parse loop body
                    auto body = parseBlock();
                    
                    // Create for loop statement node
                    return new ForLoopStatement(initialization, condition, increment, body);
                } else {
                    // It's a variable declaration with type
                    // This is not supported yet, but we'll handle it gracefully
                    throw std::runtime_error("Expected assignment operator after identifier in for loop initialization");
                }
            }
        } else {
            // It's a traditional for loop with variable declaration
            // Parse initialization as variable declaration
            Statement* initialization = parseStatement();
            
            // Expect semicolon after initialization
            consume(SEMICOLON);
            
            // Parse condition
            auto condition = parseExpression();
            
            // Expect semicolon after condition
            consume(SEMICOLON);
            
            // Parse increment
            auto increment = parseExpression();
            
            // Consume ')'
            consume(RPAREN);
            
            // Parse loop body
            auto body = parseBlock();
            
            // Create for loop statement node
            return new ForLoopStatement(initialization, condition, increment, body);
        }
    }
    
    // Check for while loop statement
    if (currentToken.type == KEYWORD && currentToken.value == "while") {
        return parseWhileLoopStatement();
    }
    
    // Check for do-while loop statement
    if (currentToken.type == KEYWORD && currentToken.value == "do") {
        return parseDoWhileLoopStatement();
    }
    
    // Check for switch statement
    if (currentToken.type == KEYWORD && currentToken.value == "switch") {
        return parseSwitchStatement();
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
            throw std::runtime_error("left side of assignment must be an identifier");
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

// Parse comparison expressions
Expression* Parser::parseComparisonExpression() {
    auto left = parseLogicalExpression();
    
    while (true) {
        if (currentToken.type == EQUAL) {
            consume(EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "==", right);
        } else if (currentToken.type == NOT_EQUAL) {
            consume(NOT_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "!=", right);
        } else if (currentToken.type == LESS_THAN) {
            consume(LESS_THAN);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "<", right);
        } else if (currentToken.type == LESS_EQUAL) {
            consume(LESS_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "<=", right);
        } else if (currentToken.type == GREATER_THAN) {
            consume(GREATER_THAN);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, ">", right);
        } else if (currentToken.type == GREATER_EQUAL) {
            consume(GREATER_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, ">=", right);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse binary expression (now delegates to comparison expression)
Expression* Parser::parseBinaryExpression() {
    return parseComparisonExpression();
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
                
                // Parse additional arguments separated by commas
                while (currentToken.type == IDENTIFIER && currentToken.value == ",") {
                    // Consume comma
                    currentToken = lexer->getNextToken();
                    
                    // Parse next argument
                    arg = parseExpression();
                    if (arg) {
                        call->arguments.push_back(arg);
                    }
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
            std::string errorMessage = "Unexpected logical operator at line " + std::to_string(currentToken.line) + " column " + std::to_string(currentToken.column);
            throw std::runtime_error(errorMessage);
        }
    }
    
    // Unexpected token
    std::string errorMessage = "Unexpected token at line " + std::to_string(currentToken.line) + " column " + std::to_string(currentToken.column);
    throw std::runtime_error(errorMessage);
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
        
        // Parse additional arguments separated by commas
        while (currentToken.type == IDENTIFIER && currentToken.value == ",") {
            // Consume comma
            consume(IDENTIFIER);
            
            // Parse next argument
            arg = parseExpression();
            if (arg) {
                call->arguments.push_back(arg);
            }
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
        case EQUAL: return "equal operator";
        case NOT_EQUAL: return "not equal operator";
        case LESS_THAN: return "less than operator";
        case LESS_EQUAL: return "less equal operator";
        case GREATER_THAN: return "greater than operator";
        case GREATER_EQUAL: return "greater equal operator";
        default: return "unknown type";
    }
}
