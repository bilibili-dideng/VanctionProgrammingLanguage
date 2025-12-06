#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <iostream>

// Constructor
Lexer::Lexer(const std::string& source) {
    this->source = source;
    this->pos = 0;
    this->line = 1;
    this->column = 1;
}

// Get next token
Token Lexer::getNextToken() {
    // Skip whitespace characters
    skipWhitespace();
    
    // Check if end of file
    if (pos >= source.length()) {
        Token token;
        token.type = EOF_TOKEN;
        token.value = "";
        token.line = line;
        token.column = column;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: EOF_TOKEN at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    char current = source[pos];
    
    if (debugMode) {
        std::cout << "[DEBUG] Lexer: Processing character '" << current << "' at line " << line << ", column " << column << std::endl;
    }
    
    // Check for comment
    if (current == '|') {
        Token token = parseComment();
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: COMMENT token: " << token.value << " at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    // Check for string literal
    if (current == '"') {
        Token token = parseStringLiteral();
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: STRING_LITERAL token: \"" << token.value << "\" at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    // Check for parentheses
    if (current == '(') {
        advance();
        Token token;
        token.type = LPAREN;
        token.value = "(";
        token.line = line;
        token.column = column - 1;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: LPAREN token at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    // Check for arithmetic operators
    if (current == '+') {
        advance();
        Token token;
        token.type = PLUS;
        token.value = "+";
        token.line = line;
        token.column = column - 1;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: PLUS token at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    if (current == '-') {
        advance();
        Token token;
        token.type = MINUS;
        token.value = "-";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '*') {
        advance();
        Token token;
        token.type = MULTIPLY;
        token.value = "*";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '/') {
        advance();
        Token token;
        token.type = DIVIDE;
        token.value = "/";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    // Check for bitwise operators
    if (current == '<') {
        advance();
        if (pos < source.length() && source[pos] == '<') {
            advance();
            Token token;
            token.type = LSHIFT;
            token.value = "<<";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = LESS_EQUAL;
            token.value = "<=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = LESS_THAN;
        token.value = "<";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '>') {
        advance();
        if (pos < source.length() && source[pos] == '>') {
            advance();
            Token token;
            token.type = RSHIFT;
            token.value = ">>";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = GREATER_EQUAL;
            token.value = ">=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = GREATER_THAN;
        token.value = ">";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == ')') {
        advance();
        Token token;
        token.type = RPAREN;
        token.value = ")";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '{') {
        advance();
        Token token;
        token.type = LBRACE;
        token.value = "{";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '}') {
        advance();
        Token token;
        token.type = RBRACE;
        token.value = "}";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '.') {
        advance();
        Token token;
        token.type = DOT;
        token.value = ".";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == ';') {
        advance();
        Token token;
        token.type = SEMICOLON;
        token.value = ";";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '=') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = EQUAL;
            token.value = "==";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = ASSIGN;
        token.value = "=";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '!') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = NOT_EQUAL;
            token.value = "!=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
    }
    if (current == '+') {
        advance();
        Token token;
        token.type = PLUS;
        token.value = "+";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '\'') {
        return parseCharLiteral();
    }
    
    // Check for comma
    if (current == ',') {
        advance();
        Token token;
        token.type = IDENTIFIER;
        token.value = ",";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    
    // Check for number literal (integer, float, or double)
    if (isdigit(current)) {
        return parseNumberLiteral();
    }
    
    // Check for identifier or keyword
    if (isalpha(current)) {
        return parseIdentifierOrKeyword();
    }
    
    // Unknown character
    char unknownChar = current;
    int unknownLine = line;
    int unknownColumn = column;
    advance();
    
    std::string errorMessage = "Unknown character '" + std::string(1, unknownChar) + "'";
    throw std::runtime_error(errorMessage);
}

// Advance one character
void Lexer::advance() {
    if (source[pos] == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    pos++;
}

// Skip whitespace characters
void Lexer::skipWhitespace() {
    while (pos < source.length() && isspace(source[pos])) {
        advance();
    }
}

// Parse identifier or keyword
Token Lexer::parseIdentifierOrKeyword() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    while (pos < source.length() && (isalnum(source[pos]) || source[pos] == '-' || source[pos] == '_')) {
        advance();
    }
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.line = start_line;
    token.column = start_column;
    
    // Check if it's a keyword
    if (value == "func" || value == "int" || value == "char" || value == "string" || 
        value == "bool" || value == "auto" || value == "define" || value == "true" || value == "false" ||
        value == "float" || value == "double" || value == "AND" || value == "OR" || value == "XOR" ||
        // Control flow keywords
        value == "if" || value == "else" || value == "else-if" || value == "for" || value == "while" || value == "do" ||
        value == "switch" || value == "case" || value == "in" || value == "return") {
        token.type = KEYWORD;
        token.value = value;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: KEYWORD token: " << token.value << " at line " << token.line << ", column " << token.column << std::endl;
        }
    } else {
        token.type = IDENTIFIER;
        token.value = value;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: IDENTIFIER token: " << token.value << " at line " << token.line << ", column " << token.column << std::endl;
        }
    }
    
    return token;
}

// Parse character literal
Token Lexer::parseCharLiteral() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip opening quote
    advance();
    
    // Read until closing quote
    while (pos < source.length() && source[pos] != '\'') {
        advance();
    }
    
    // Skip closing quote
    advance();
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.type = CHAR_LITERAL;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Parse number literal (integer, float, or double)
Token Lexer::parseNumberLiteral() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    bool hasDecimal = false;
    
    // Parse digits before decimal point
    while (pos < source.length() && isdigit(source[pos])) {
        advance();
    }
    
    // Check for decimal point
    if (pos < source.length() && source[pos] == '.') {
        hasDecimal = true;
        advance();
        
        // Parse digits after decimal point
        while (pos < source.length() && isdigit(source[pos])) {
            advance();
        }
    }
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.line = start_line;
    token.column = start_column;
    
    if (hasDecimal) {
        // Check if it's a float (ends with 'f' or 'F')
        if (pos < source.length() && (source[pos] == 'f' || source[pos] == 'F')) {
            token.type = FLOAT_LITERAL;
            token.value = value;
            advance();
        } 
        // Check if it's a double (ends with 'd' or 'D')
        else if (pos < source.length() && (source[pos] == 'd' || source[pos] == 'D')) {
            token.type = DOUBLE_LITERAL;
            token.value = value;
            advance();
        } else {
            // Default to double for decimal numbers
            token.type = DOUBLE_LITERAL;
            token.value = value;
        }
    } else {
        token.type = INTEGER_LITERAL;
        token.value = value;
    }
    
    return token;
}

// Parse comment
Token Lexer::parseComment() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip comment until end of line or closing brace
    while (pos < source.length() && source[pos] != '\n' && source[pos] != '}') {
        advance();
    }
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.type = COMMENT;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Parse string literal
Token Lexer::parseStringLiteral() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip opening quote
    advance();
    
    // Read until closing quote
    while (pos < source.length() && source[pos] != '"') {
        advance();
    }
    
    // Skip closing quote
    advance();
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.type = STRING_LITERAL;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}
