#include "parser.h"
#include "error.h"
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
    // Parse all declarations until end of file
    while (currentToken.type != EOF_TOKEN) {
        if (currentToken.type == KEYWORD && currentToken.value == "func") {
            if (parseFunction()) {
                // Check if it's main function
                if (functionName == "main") {
                    return true;
                }
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "namespace") {
            // Skip namespace declaration
            consume(KEYWORD);
            consume(IDENTIFIER);
            consume(LBRACE);
            
            // Skip all content until matching right brace
            int braceCount = 1;
            while (currentToken.type != EOF_TOKEN && braceCount > 0) {
                if (currentToken.type == LBRACE) {
                    braceCount++;
                } else if (currentToken.type == RBRACE) {
                    braceCount--;
                }
                currentToken = lexer->getNextToken();
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "class") {
            // Skip class declaration
            consume(KEYWORD);
            consume(IDENTIFIER);
            consume(LPAREN);
            
            // Skip base class if exists
            if (currentToken.type == IDENTIFIER) {
                consume(IDENTIFIER);
            }
            
            consume(RPAREN);
            consume(LBRACE);
            
            // Skip all content until matching right brace
            int braceCount = 1;
            while (currentToken.type != EOF_TOKEN && braceCount > 0) {
                if (currentToken.type == LBRACE) {
                    braceCount++;
                } else if (currentToken.type == RBRACE) {
                    braceCount--;
                }
                currentToken = lexer->getNextToken();
            }
        } else {
            // If it's not a function, namespace, or class declaration, skip this token and continue
            currentToken = lexer->getNextToken();
        }
    }
    
    return false;
}

// Parse namespace declaration and generate AST
NamespaceDeclaration* Parser::parseNamespaceDeclarationAST() {
    // Consume 'namespace' keyword
    consume(KEYWORD);
    
    // Parse namespace name
    std::string name = currentToken.value;
    consume(IDENTIFIER);
    
    // Parse left brace
    consume(LBRACE);
    
    // Create namespace declaration node
    auto ns = new NamespaceDeclaration(name);
    
    // Parse declarations inside namespace until right brace
    while (currentToken.type != RBRACE && currentToken.type != EOF_TOKEN) {
        if (currentToken.type == KEYWORD && currentToken.value == "func") {
            auto func = parseFunctionAST();
            if (func) {
                ns->declarations.push_back(func);
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "namespace") {
            auto nestedNs = parseNamespaceDeclarationAST();
            if (nestedNs) {
                ns->declarations.push_back(nestedNs);
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "class") {
            auto cls = parseClassDeclarationAST();
            if (cls) {
                ns->declarations.push_back(cls);
            }
        } else {
            // If it's not a function, namespace, or class declaration, skip this token and continue
            currentToken = lexer->getNextToken();
        }
    }
    
    // Parse right brace
    consume(RBRACE);
    
    return ns;
}

// Parse class declaration and generate AST
ClassDeclaration* Parser::parseClassDeclarationAST() {
    // Consume 'class' keyword
    consume(KEYWORD);
    
    // Parse class name
    std::string name = currentToken.value;
    consume(IDENTIFIER);
    
    // Parse left parenthesis for inheritance
    consume(LPAREN);
    
    // Parse base class (optional)
    std::string baseClassName = "";
    if (currentToken.type == IDENTIFIER) {
        baseClassName = currentToken.value;
        consume(IDENTIFIER);
    }
    
    // Parse right parenthesis
    consume(RPAREN);
    
    // Parse left brace
    consume(LBRACE);
    
    // Create class declaration node
    auto cls = new ClassDeclaration(name, baseClassName);
    
    // Parse declarations inside class until right brace
    while (currentToken.type != RBRACE && currentToken.type != EOF_TOKEN) {
        // Parse init method or instance method
        if (currentToken.type == KEYWORD && currentToken.value == "instance") {
            consume(KEYWORD); // Consume instance
            
            if (currentToken.type == DOT) {
                consume(DOT); // Consume dot
                
                if (currentToken.type == KEYWORD && currentToken.value == "init") {
                    consume(KEYWORD); // Consume init
                    
                    // Parse method parameters
                    consume(LPAREN);
                    
                    // Create init method declaration
                    auto initMethod = new InstanceMethodDeclaration(name, "init", "void");
                    
                    // Parse parameters - include the instance parameter
                    bool firstParam = true;
                    while ((currentToken.type == IDENTIFIER || currentToken.type == KEYWORD) && 
                           (currentToken.value == "instance" || currentToken.type == IDENTIFIER)) {
                        std::string paramName = currentToken.value;
                        consume(currentToken.type);
                        initMethod->parameters.push_back(FunctionParameter(paramName));
                        
                        if (currentToken.type == COMMA) {
                            consume(COMMA);
                        } else {
                            break;
                        }
                        firstParam = false;
                    }
                    
                    // Parse right parenthesis
                    consume(RPAREN);
                    
                    // Parse method body
                    consume(LBRACE);
                    initMethod->body = parseFunctionBodyAST();
                    consume(RBRACE);
                    
                    cls->initMethod = initMethod;
                } else {
                    // It's an instance method, not init
                    std::string methodName = currentToken.value;
                    consume(IDENTIFIER);
                    
                    // Parse method parameters
                    consume(LPAREN);
                    
                    // Create instance method declaration
                    auto method = new InstanceMethodDeclaration(name, methodName, "void");
                    
                    // Parse parameters
                    bool firstParam = true;
                    while (currentToken.type != RPAREN) {
                        // Check if current token is a valid parameter name
                        if (currentToken.type == IDENTIFIER || currentToken.type == KEYWORD) {
                            std::string paramName = currentToken.value;
                            consume(currentToken.type);
                            
                            // Add parameter to the list if it's not "instance"
                            if (paramName != "instance") {
                                method->parameters.push_back(FunctionParameter(paramName));
                            }
                            
                            // Check if there's a comma after the parameter
                            if (currentToken.type == COMMA) {
                                consume(COMMA);
                            } else if (currentToken.type != RPAREN) {
                                // Invalid token, break the loop
                                break;
                            }
                        } else {
                            // Invalid token, break the loop
                            break;
                        }
                        firstParam = false;
                    }
                    
                    // Parse right parenthesis
                    consume(RPAREN);
                    
                    // Parse method body
                    consume(LBRACE);
                    method->body = parseFunctionBodyAST();
                    consume(RBRACE);
                    
                    cls->instanceMethods.push_back(method);
                }
            }
        }
        // Parse class method
        else if (currentToken.type == KEYWORD && currentToken.value == "class") {
            // Consume class keyword
            consume(KEYWORD);
            // Consume dot
            consume(DOT);
            
            // Parse method name
            std::string methodName = currentToken.value;
            consume(IDENTIFIER);
            
            // Parse method parameters
            consume(LPAREN);
            
            // Create class method declaration
            auto method = new ClassMethodDeclaration(name, methodName, "void");
            
            // Parse parameters
                    while (currentToken.type == IDENTIFIER) {
                        std::string paramName = currentToken.value;
                        consume(IDENTIFIER);
                        method->parameters.push_back(FunctionParameter(paramName));
                        
                        if (currentToken.type == COMMA) {
                            consume(COMMA);
                        } else {
                            break;
                        }
                    }
            
            // Parse right parenthesis
            consume(RPAREN);
            
            // Parse method body
            consume(LBRACE);
            method->body = parseFunctionBodyAST();
            consume(RBRACE);
            
            cls->methods.push_back(method);
        }

        // Skip other tokens
        else {
            currentToken = lexer->getNextToken();
        }
    }
    
    // Parse right brace
    consume(RBRACE);
    
    return cls;
}

// Parse program and generate AST
Program* Parser::parseProgramAST() {
    auto program = new Program();
    
    // Parse all declarations until end of file
    while (currentToken.type != EOF_TOKEN) {
        if (currentToken.type == KEYWORD && currentToken.value == "func") {
            auto func = parseFunctionAST();
            if (func) {
                program->declarations.push_back(func);
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "namespace") {
            auto ns = parseNamespaceDeclarationAST();
            if (ns) {
                program->declarations.push_back(ns);
            }
        } else if (currentToken.type == KEYWORD && currentToken.value == "class") {
            auto cls = parseClassDeclarationAST();
            if (cls) {
                program->declarations.push_back(cls);
            }
        } else {
            // If it's not a function, namespace, or class declaration, skip this token and continue
            currentToken = lexer->getNextToken();
        }
    }
    
    return program;
}

// Consume current token
void Parser::consume(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        currentToken = lexer->getNextToken();
    } else {
        std::string errorMessage = "expected " + tokenTypeToString(expectedType) + ", but got " + tokenTypeToString(currentToken.type);
        throw vanction_error::SyntaxError(errorMessage, currentToken.line, currentToken.column);
    }
}

// Parse try-happen statement for error handling
Statement* Parser::parseTryHappenStatement() {
    // Consume 'try' keyword
    consume(KEYWORD);
    
    // Parse try block
    auto tryBody = parseBlock();
    
    // Expect 'happen' keyword
    if (currentToken.type != KEYWORD || currentToken.value != "happen") {
        throw std::runtime_error("Expected 'happen' keyword after try block");
    }
    
    // Consume 'happen' keyword
    consume(KEYWORD);
    
    // Expect left parenthesis
    consume(LPAREN);
    
    // Parse error type
    std::string errorType;
    if (currentToken.type == IDENTIFIER) {
        errorType = currentToken.value;
        consume(IDENTIFIER);
    } else {
        throw std::runtime_error("Expected error type identifier");
    }
    
    // Expect right parenthesis
    consume(RPAREN);
    
    // Expect 'as' keyword
    if (currentToken.type != KEYWORD || currentToken.value != "as") {
        throw std::runtime_error("Expected 'as' keyword after error type");
    }
    
    // Consume 'as' keyword
    consume(KEYWORD);
    
    // Parse error variable name
    std::string errorVariableName;
    if (currentToken.type == IDENTIFIER) {
        errorVariableName = currentToken.value;
        consume(IDENTIFIER);
    } else {
        throw std::runtime_error("Expected error variable name");
    }
    
    // Parse happen block
    auto happenBody = parseBlock();
    
    // Create try-happen statement node
    return new TryHappenStatement(tryBody, errorType, errorVariableName, happenBody);
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
    
    // Parse function name - no return type, no parameter types
    std::string returnType = "auto"; // Default return type is auto
    std::string funcName = currentToken.value;
    consume(currentToken.type);
    
    // Parse left parenthesis
    consume(LPAREN);
    
    // Parse parameters
    std::vector<FunctionParameter> parameters;
    if (currentToken.type != RPAREN) {
        // Parse first parameter - only parameter name, no type
        std::string paramName = currentToken.value;
        consume(IDENTIFIER);
        
        parameters.emplace_back(paramName);
        
        // Parse additional parameters
            while (currentToken.type == COMMA) {
                consume(COMMA); // Consume comma
                
                // Parse parameter name - only parameter name, no type
                std::string paramName = currentToken.value;
                consume(IDENTIFIER);
                
                parameters.emplace_back(paramName);
            }
    }
    
    // Parse right parenthesis
    consume(RPAREN);
    
    // Parse left brace
    consume(LBRACE);
    
    // Parse function body
    auto body = parseFunctionBodyAST();
    
    // Parse right brace
    consume(RBRACE);
    
    // Create function declaration node
    auto func = new FunctionDeclaration(returnType, funcName);
    func->parameters = std::move(parameters);
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
        } else if (currentToken.type == KEYWORD && currentToken.value == "func") {
            // Parse nested function declaration
            auto func = parseFunctionAST();
            if (func) {
                body.push_back(func);
            }
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
    
    // Check for try-happen statement
    if (currentToken.type == KEYWORD && currentToken.value == "try") {
        return parseTryHappenStatement();
    }
    
    // Check for return statement
    if (currentToken.type == KEYWORD && currentToken.value == "return") {
        // Consume 'return' keyword
        consume(KEYWORD);
        
        // Parse optional expression
        Expression* expr = nullptr;
        if (currentToken.type != SEMICOLON) {
            expr = parseExpression();
        }
        
        // Expect semicolon
        consume(SEMICOLON);
        
        // Create return statement
        return new ReturnStatement(expr);
    }
    
    // Parse expression
    auto expr = parseExpression();
    if (!expr) {
        return nullptr;
    }
    
    // Save the expression's line and column for error reporting
    int exprLine = expr->getLine();
    int exprColumn = expr->getColumn();
    
    // Expect semicolon
    try {
        consume(SEMICOLON);
    } catch (const vanction_error::SyntaxError& e) {
        // If we got a comment instead of semicolon, use the expression's line and column
        if (currentToken.type == COMMENT) {
            std::string errorMessage = "expected semicolon, but got comment";
            throw vanction_error::SyntaxError(errorMessage, exprLine, exprColumn);
        }
        // Re-throw other syntax errors
        throw;
    }
    
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
    
    // Handle compound assignment operators
    if (currentToken.type == ASSIGN) {
        int line = left->getLine();
        int column = left->getColumn();
        consume(ASSIGN);
        auto right = parseAssignmentExpression();
        return new AssignmentExpression(left, right, line, column);
    } else if (currentToken.type == PLUS_ASSIGN) {
        consume(PLUS_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "+", right);
    } else if (currentToken.type == MINUS_ASSIGN) {
        consume(MINUS_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "-", right);
    } else if (currentToken.type == MULTIPLY_ASSIGN) {
        consume(MULTIPLY_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "*", right);
    } else if (currentToken.type == DIVIDE_ASSIGN) {
        consume(DIVIDE_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "/", right);
    } else if (currentToken.type == MODULO_ASSIGN) {
        consume(MODULO_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "%", right);
    } else if (currentToken.type == LSHIFT_ASSIGN) {
        consume(LSHIFT_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "<<", right);
    } else if (currentToken.type == RSHIFT_ASSIGN) {
        consume(RSHIFT_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, ">>", right);
    } else if (currentToken.type == AND_ASSIGN) {
        consume(AND_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "&", right);
    } else if (currentToken.type == OR_ASSIGN) {
        consume(OR_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "|", right);
    } else if (currentToken.type == XOR_ASSIGN) {
        consume(XOR_ASSIGN);
        auto right = parseAssignmentExpression();
        return new BinaryExpression(left, "^", right);
    }
    
    return left;
}

// Parse multiplication and division expressions
Expression* Parser::parseMultiplicativeExpression() {
    auto left = parsePrimaryExpression();
    
    while (true) {
        if (currentToken.type == MULTIPLY) {
        int line = currentToken.line;
        int column = currentToken.column;
        consume(MULTIPLY);
        auto right = parsePrimaryExpression();
        left = new BinaryExpression(left, "*", right, line, column);
    } else if (currentToken.type == DIVIDE) {
        int line = currentToken.line;
        int column = currentToken.column;
        consume(DIVIDE);
        auto right = parsePrimaryExpression();
        left = new BinaryExpression(left, "/", right, line, column);
    } else if (currentToken.type == MODULO) {
        int line = currentToken.line;
        int column = currentToken.column;
        consume(MODULO);
        auto right = parsePrimaryExpression();
        left = new BinaryExpression(left, "%", right, line, column);
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
            int line = currentToken.line;
            int column = currentToken.column;
            consume(PLUS);
            auto right = parseMultiplicativeExpression();
            left = new BinaryExpression(left, "+", right, line, column);
        } else if (currentToken.type == MINUS) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(MINUS);
            auto right = parseMultiplicativeExpression();
            left = new BinaryExpression(left, "-", right, line, column);
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
            int line = currentToken.line;
            int column = currentToken.column;
            consume(LSHIFT);
            auto right = parseAdditiveExpression();
            left = new BinaryExpression(left, "<<", right, line, column);
        } else if (currentToken.type == RSHIFT) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(RSHIFT);
            auto right = parseAdditiveExpression();
            left = new BinaryExpression(left, ">>", right, line, column);
        } else {
            break;
        }
    }
    
    return left;
}

// Parse logical expressions (&, |, ^)
Expression* Parser::parseLogicalExpression() {
    auto left = parseBitShiftExpression();
    
    while (true) {
        if (currentToken.type == BITWISE_AND) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(BITWISE_AND);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "&", right, line, column);
        } else if (currentToken.type == BITWISE_OR) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(BITWISE_OR);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "|", right, line, column);
        } else if (currentToken.type == XOR) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(XOR);
            auto right = parseBitShiftExpression();
            left = new BinaryExpression(left, "^", right, line, column);
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
            int line = currentToken.line;
            int column = currentToken.column;
            consume(EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "==", right, line, column);
        } else if (currentToken.type == NOT_EQUAL) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(NOT_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "!=", right, line, column);
        } else if (currentToken.type == LESS_THAN) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(LESS_THAN);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "<", right, line, column);
        } else if (currentToken.type == LESS_EQUAL) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(LESS_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, "<=", right, line, column);
        } else if (currentToken.type == GREATER_THAN) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(GREATER_THAN);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, ">", right, line, column);
        } else if (currentToken.type == GREATER_EQUAL) {
            int line = currentToken.line;
            int column = currentToken.column;
            consume(GREATER_EQUAL);
            auto right = parseLogicalExpression();
            left = new BinaryExpression(left, ">=", right, line, column);
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
    // Check for class method call (e.g., class.method())
    if (currentToken.type == KEYWORD && currentToken.value == "class") {
        // Create an Identifier object for "class"
        auto ident = new Identifier("class", currentToken.line, currentToken.column);
        
        // Consume the "class" keyword
        currentToken = lexer->getNextToken();
        
        // Check if it's a class method call (e.g., class.method())
        if (currentToken.type == DOT) {
            // Consume dot
            currentToken = lexer->getNextToken();
            
            // Parse method name
            std::string methodName = currentToken.value;
            int methodLine = currentToken.line;
            int methodColumn = currentToken.column;
            currentToken = lexer->getNextToken();
            
            // It should be a method call
            if (currentToken.type != LPAREN) {
                throw std::runtime_error("Expected left parenthesis after class method name");
            }
            
            // Consume left parenthesis
            consume(LPAREN);
            
            // Create function call node
            auto call = new FunctionCall("class", methodName, methodLine, methodColumn);
            
            // Parse arguments
            if (currentToken.type != RPAREN) {
                auto arg = parseExpression();
                if (arg) {
                    call->arguments.push_back(arg);
                }
                
                // Parse additional arguments separated by commas
                while (currentToken.type == COMMA) {
                    // Consume comma
                    consume(COMMA);
                    
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
        // Otherwise, it's just a regular identifier "class"
        return ident;
    }
    
    // Check for instance creation or instance access (e.g., instance ClassName() or instance.member)
    if (currentToken.type == KEYWORD && currentToken.value == "instance") {
        // Create an Identifier object for "instance"
        auto ident = new Identifier("instance");
        
        // Consume the "instance" keyword
        currentToken = lexer->getNextToken();
        
        // Check if it's an instance creation expression (e.g., instance ClassName() or instance Namespace:ClassName())
        if (currentToken.type == IDENTIFIER) {
            // It's an instance creation expression
            std::string className = currentToken.value;
            consume(IDENTIFIER);
            
            // Check if it's a namespace:Class syntax
            if (currentToken.type == COLON) {
                // Save the namespace name
                std::string namespaceName = className;
                
                // Consume colon
                currentToken = lexer->getNextToken();
                
                // Parse actual class name
                std::string actualClassName = currentToken.value;
                consume(IDENTIFIER);
                
                // Consume left parenthesis
                consume(LPAREN);
                
                // Create instance creation expression with namespace
                auto instanceExpr = new InstanceCreationExpression(actualClassName, namespaceName);
                
                // Parse arguments
                if (currentToken.type != RPAREN) {
                    auto arg = parseExpression();
                    if (arg) {
                        instanceExpr->arguments.push_back(arg);
                    }
                    
                    // Parse additional arguments separated by commas
                    while (currentToken.type == COMMA) {
                        // Consume comma
                        consume(COMMA);
                        
                        // Parse next argument
                        arg = parseExpression();
                        if (arg) {
                            instanceExpr->arguments.push_back(arg);
                        }
                    }
                }
                
                // Expect right parenthesis
                consume(RPAREN);
                
                return instanceExpr;
            }
            
            // Parse arguments
            consume(LPAREN);
            
            // Create instance creation expression
            auto instanceExpr = new InstanceCreationExpression(className);
            
            // Parse arguments
            if (currentToken.type != RPAREN) {
                auto arg = parseExpression();
                if (arg) {
                    instanceExpr->arguments.push_back(arg);
                }
                
                // Parse additional arguments separated by commas
                while (currentToken.type == COMMA) {
                    // Consume comma
                    consume(COMMA);
                    
                    // Parse next argument
                    arg = parseExpression();
                    if (arg) {
                        instanceExpr->arguments.push_back(arg);
                    }
                }
            }
            
            // Expect right parenthesis
            consume(RPAREN);
            
            return instanceExpr;
        }
        // Check if it's an instance access expression (e.g., instance.member)
        else if (currentToken.type == DOT) {
            // Consume dot
            currentToken = lexer->getNextToken();
            
            // Parse member name
            std::string memberName = currentToken.value;
            currentToken = lexer->getNextToken();
            
            // Check if it's a method call (e.g., instance.method())
            if (currentToken.type == LPAREN) {
                // Consume left parenthesis
                consume(LPAREN);
                
                // Create function call node
                auto call = new FunctionCall("instance", memberName);
                
                // Parse arguments
                if (currentToken.type != RPAREN) {
                    auto arg = parseExpression();
                    if (arg) {
                        call->arguments.push_back(arg);
                    }
                    
                    // Parse additional arguments separated by commas
                    while (currentToken.type == COMMA) {
                        // Consume comma
                        consume(COMMA);
                        
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
                // It's a simple instance member access
                return new InstanceAccessExpression(ident, memberName);
            }
        }
        // Otherwise, it's just a regular identifier "instance"
        return ident;
    }
    
    // Check for identifier or instance keyword
    if (currentToken.type == IDENTIFIER || (currentToken.type == KEYWORD && currentToken.value == "instance")) {
        std::string name = currentToken.value;
        int line = currentToken.line;
        int column = currentToken.column;
        currentToken = lexer->getNextToken();
        
        // Check if it's a namespace access (e.g., namespace:member)
        if (currentToken.type == COLON) {
            // It's a namespace access, consume colon
            currentToken = lexer->getNextToken();
            
            // Parse member name
            std::string memberName = currentToken.value;
            currentToken = lexer->getNextToken();
            
            // Check if it's a function call (e.g., namespace:func())
            if (currentToken.type == LPAREN) {
                // It's a function call using namespace access
                consume(LPAREN);
                
                // Create function call node
            auto call = new FunctionCall(name, memberName, line, column);
                
                // Parse arguments
                if (currentToken.type != RPAREN) {
                    auto arg = parseExpression();
                    if (arg) {
                        call->arguments.push_back(arg);
                    }
                    
                    // Parse additional arguments separated by commas
                    while (currentToken.type == COMMA) {
                        // Consume comma
                        consume(COMMA);
                        
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
                // It's a simple namespace access
                return new NamespaceAccess(name, memberName);
            }
        }
        // Check if it's an instance access (e.g., obj.member or obj.method())
        else if (currentToken.type == DOT) {
            // Consume dot
            currentToken = lexer->getNextToken();
            
            // Parse member name
            std::string memberName = currentToken.value;
            currentToken = lexer->getNextToken();
            
            // Check if it's a method call (e.g., obj.method())
            if (currentToken.type == LPAREN) {
                // Consume left parenthesis
                consume(LPAREN);
                
                // Create function call node
                auto call = new FunctionCall(name, memberName);
                
                // Parse arguments
                if (currentToken.type != RPAREN) {
                    auto arg = parseExpression();
                    if (arg) {
                        call->arguments.push_back(arg);
                    }
                    
                    // Parse additional arguments separated by commas
                    while (currentToken.type == COMMA) {
                        // Consume comma
                        consume(COMMA);
                        
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
                // It's a simple instance member access
                return new InstanceAccessExpression(new Identifier(name), memberName);
            }
        }
        // Check if it's a regular function call (e.g., myFunction())
        else if (currentToken.type == LPAREN) {
            // It's a regular function call
            consume(LPAREN);
            
            // Create function call node (use empty object name for regular function calls)
            auto call = new FunctionCall("", name);
            
            // Parse arguments
            if (currentToken.type != RPAREN) {
                auto arg = parseExpression();
                if (arg) {
                    call->arguments.push_back(arg);
                }
                
                // Parse additional arguments separated by commas
                while (currentToken.type == COMMA) {
                    // Consume comma
                    consume(COMMA);
                    
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
        else {
            // It's a simple identifier
            return new Identifier(name, line, column);
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
    std::string tokenValue = currentToken.value;
    std::string type = "normal";
    std::string value;
    
    // Check for prefix and remove it along with quotes
    if (tokenValue.size() >= 3 && (tokenValue[0] == 'r' || tokenValue[0] == 'f') && tokenValue[1] == '"') {
        // Has prefix
        type = (tokenValue[0] == 'r') ? "raw" : "format";
        // Remove prefix and quotes
        value = tokenValue.substr(2, tokenValue.size() - 3);
    } else if (tokenValue.size() >= 2) {
        // Normal string without prefix
        value = tokenValue.substr(1, tokenValue.size() - 2);
    }
    
    // Process escape sequences for non-raw strings
    if (type != "raw") {
        std::string processedValue;
        for (size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '\\' && i + 1 < value.size()) {
                // Handle escape sequences
                char nextChar = value[i + 1];
                switch (nextChar) {
                    case 'n': processedValue += '\n'; break;
                    case 't': processedValue += '\t'; break;
                    case 'r': processedValue += '\r'; break;
                    case '"': processedValue += '"'; break;
                    case '\\': processedValue += '\\'; break;
                    default: processedValue += nextChar; break;
                }
                i++; // Skip the next character
            } else {
                processedValue += value[i];
            }
        }
        value = processedValue;
    }
    
    currentToken = lexer->getNextToken();
    return new StringLiteral(value, type);
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
        while (currentToken.type == COMMA) {
            // Consume comma
            consume(COMMA);
            
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
        case COLON: return "colon";
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
