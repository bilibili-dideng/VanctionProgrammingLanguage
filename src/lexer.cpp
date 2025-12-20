#include "lexer.h"
#include "error.h"
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
    
    // Check for comment types
    // Single-line comment: || ...
    if (current == '|' && pos + 1 < source.length()) {
        char nextChar = source[pos + 1];
        if (nextChar == '|') {
            // Single-line comment: || ...
            advance(); // Consume first |
            advance(); // Consume second |
            return parseComment();
        } 
        // Multi-line comment: |* ... *|
        else if (nextChar == '*') {
            advance(); // Consume |
            advance(); // Consume *
            return parseMultiLineComment();
        }
        // Doc comment: |\ ... /|
        else if (nextChar == '\\') {
            advance(); // Consume |
            advance(); // Consume \
            // Parse doc comment content until /|
            size_t start = pos;
            int start_line = line;
            int start_column = column;
            
            // Find closing /|
            while (pos + 1 < source.length()) {
                if (source[pos] == '/' && source[pos + 1] == '|') {
                    break;
                }
                // Advance character by character
                if (source[pos] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                pos++;
            }
            
            // Consume closing /|
            if (pos + 1 < source.length()) {
                // Consume /
                column++;
                pos++;
                // Consume |
                column++;
                pos++;
            }
            
            // Create comment token
            Token token;
            token.type = COMMENT;
            token.value = source.substr(start, pos - start - 2);
            token.line = start_line;
            token.column = start_column;
            return token;
        }
    }
    
    // Check for string literal (with optional r or f prefix)
    if (current == '"' || (pos + 1 < source.length() && (current == 'r' || current == 'f') && source[pos + 1] == '"')) {
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
    
    // Check for left bracket
    if (current == '[') {
        advance();
        Token token;
        token.type = LBRACKET;
        token.value = "[";
        token.line = line;
        token.column = column - 1;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: LBRACKET token at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    // Check for right bracket
    if (current == ']') {
        advance();
        Token token;
        token.type = RBRACKET;
        token.value = "]";
        token.line = line;
        token.column = column - 1;
        if (debugMode) {
            std::cout << "[DEBUG] Lexer: RBRACKET token at line " << token.line << ", column " << token.column << std::endl;
        }
        return token;
    }
    
    // Check for arithmetic operators
    if (current == '+') {
        advance();
        if (pos < source.length() && source[pos] == '+') {
            advance();
            Token token;
            token.type = INCREMENT;
            token.value = "++";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = PLUS_ASSIGN;
            token.value = "+=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
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
        if (pos < source.length() && source[pos] == '-') {
            advance();
            Token token;
            token.type = DECREMENT;
            token.value = "--";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = MINUS_ASSIGN;
            token.value = "-=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = MINUS;
        token.value = "-";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '*') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = MULTIPLY_ASSIGN;
            token.value = "*=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = MULTIPLY;
        token.value = "*";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '/') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = DIVIDE_ASSIGN;
            token.value = "/=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = DIVIDE;
        token.value = "/";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '%') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = MODULO_ASSIGN;
            token.value = "%=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = MODULO;
        token.value = "%";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    // Check for bitwise operators
    if (current == '<') {
        advance();
        if (pos < source.length() && source[pos] == '<') {
            advance();
            if (pos < source.length() && source[pos] == '=') {
                advance();
                Token token;
                token.type = LSHIFT_ASSIGN;
                token.value = "<<=";
                token.line = line;
                token.column = column - 3;
                return token;
            }
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
            if (pos < source.length() && source[pos] == '=') {
                advance();
                Token token;
                token.type = RSHIFT_ASSIGN;
                token.value = ">>=";
                token.line = line;
                token.column = column - 3;
                return token;
            }
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
    if (current == '&') {
        advance();
        if (pos < source.length() && source[pos] == '&') {
            advance();
            Token token;
            token.type = AND;
            token.value = "&&";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = AND_ASSIGN;
            token.value = "&=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = BITWISE_AND;
        token.value = "&";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '|') {
        advance();
        if (pos < source.length() && source[pos] == '|') {
            advance();
            Token token;
            token.type = OR;
            token.value = "||";
            token.line = line;
            token.column = column - 2;
            return token;
        } else if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = OR_ASSIGN;
            token.value = "|=";
            token.line = line;
            token.column = column - 2;
            return token;
        } else {
            // Single | is now bitwise OR operator
            Token token;
            token.type = BITWISE_OR;
            token.value = "|";
            token.line = line;
            token.column = column - 1;
            return token;
        }
    }
    if (current == '^') {
        advance();
        if (pos < source.length() && source[pos] == '=') {
            advance();
            Token token;
            token.type = XOR_ASSIGN;
            token.value = "^=";
            token.line = line;
            token.column = column - 2;
            return token;
        }
        Token token;
        token.type = XOR;
        token.value = "^";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    if (current == '~') {
        advance();
        Token token;
        token.type = BITWISE_NOT;
        token.value = "~";
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
    
    // Check for colon (namespace access operator)
    if (current == ':') {
        advance();
        Token token;
        token.type = COLON;
        token.value = ":";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    
    // Check for comma
    if (current == ',') {
        advance();
        Token token;
        token.type = COMMA;
        token.value = ",";
        token.line = line;
        token.column = column - 1;
        return token;
    }
    
    // Check for number literal (integer, float, or double)
    if (isdigit(current) || (current == '-' && pos + 1 < source.length() && isdigit(source[pos + 1]))) {
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
    throw vanction_error::TokenError(errorMessage, unknownLine, unknownColumn);
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
        value == "float" || value == "double" || value == "List" || value == "HashMap" ||
        // Variable declaration keywords
        value == "var" || value == "immut" ||
        // Control flow keywords
        value == "if" || value == "else" || value == "else-if" || value == "for" || value == "while" || value == "do" ||
        value == "switch" || value == "case" || value == "in" || value == "return" || value == "namespace" ||
        // Error handling keywords
        value == "try" || value == "happen" || value == "as" ||
        // Import keywords
        value == "import" || value == "using" || value == "to" ||
        // OOP keywords
        value == "class" || value == "instance" || value == "init" ||
        // Lambda expression keyword
        value == "lambda") {
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
    bool isNegative = false;
    
    // Check for negative sign
    if (source[pos] == '-') {
        isNegative = true;
        advance();
    }
    
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
    
    // Check if it's a float (ends with 'f' or 'F') - works for both decimal and integer literals
    if (pos < source.length() && (source[pos] == 'f' || source[pos] == 'F')) {
        token.type = FLOAT_LITERAL;
        token.value = value;
        advance();
    } 
    // Check if it's a double (ends with 'd' or 'D') - works for both decimal and integer literals
    else if (pos < source.length() && (source[pos] == 'd' || source[pos] == 'D')) {
        token.type = DOUBLE_LITERAL;
        token.value = value;
        advance();
    } else if (hasDecimal) {
        // Default to double for decimal numbers
        token.type = DOUBLE_LITERAL;
        token.value = value;
    } else {
        token.type = INTEGER_LITERAL;
        token.value = value;
    }
    
    return token;
}

// Parse single-line comment
Token Lexer::parseComment() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip comment until end of line, handling UTF-8 characters properly
    while (pos < source.length()) {
        // Check if we've reached the end of the line
        if (source[pos] == '\n') {
            break;
        }
        // Advance past the current character, regardless of its UTF-8 encoding
        // This ensures we don't get stuck on multi-byte characters
        pos++;
        column++;
    }
    
    std::string value = source.substr(start, pos - start);
    
    Token token;
    token.type = COMMENT;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Parse multi-line comment |* ... *|
Token Lexer::parseMultiLineComment() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip comment until closing *|
    while (pos + 1 < source.length()) {
        if (source[pos] == '*' && source[pos + 1] == '|') {
            break;
        }
        // Directly advance pos and column to handle UTF-8 characters properly
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
    
    // Consume closing *|
    if (pos + 1 < source.length()) {
        // Consume *
        column++;
        pos++;
        // Consume |
        column++;
        pos++;
    }
    
    std::string value = source.substr(start, pos - start - 2); // -2 to exclude the closing *|
    
    Token token;
    token.type = COMMENT;
    token.value = value;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Parse doc comment |\ ... /|
Token Lexer::parseDocComment() {
    size_t start = pos;
    int start_line = line;
    int start_column = column;
    
    // Skip comment until closing /| (correct end delimiter)
    while (pos + 1 < source.length()) {
        if (source[pos] == '/' && source[pos + 1] == '|') {
            break;
        }
        // Handle newline for proper line counting
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
    
    // Extract comment content
    std::string value;
    if (pos > start) {
        value = source.substr(start, pos - start);
    }
    
    // Consume closing delimiter /|
    if (pos + 1 < source.length()) {
        // Consume /
        column++;
        pos++;
        // Consume |
        column++;
        pos++;
    }
    
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
    
    // Check for Python-style multi-line comment: """
    if (pos + 2 < source.length() && source[pos] == '"' && source[pos + 1] == '"' && source[pos + 2] == '"') {
        // This is a multi-line comment, not a string literal
        advance(); // Consume first "
        advance(); // Consume second "
        advance(); // Consume third "
        
        // Skip comment until closing """
        while (pos + 2 < source.length()) {
            if (source[pos] == '"' && source[pos + 1] == '"' && source[pos + 2] == '"') {
                break;
            }
            advance();
        }
        
        // Consume closing """
        if (pos + 2 < source.length()) {
            advance(); // Consume first "
            advance(); // Consume second "
            advance(); // Consume third "
        }
        
        std::string value = source.substr(start + 3, pos - start - 6); // +3 to skip opening """, -6 to exclude both opening and closing
        
        Token token;
        token.type = COMMENT;
        token.value = value;
        token.line = start_line;
        token.column = start_column;
        
        return token;
    }
    
    // Check for string prefix (r or f)
    std::string prefix;
    if (pos + 1 < source.length() && (source[pos] == 'r' || source[pos] == 'f') && source[pos + 1] == '"') {
        prefix = std::string(1, source[pos]);
        advance(); // Skip prefix
    }
    
    // Skip opening quote
    advance();
    
    // Read until closing quote, handling escape sequences for non-raw strings
    while (pos < source.length() && source[pos] != '"') {
        // Handle escape sequences for non-raw strings
        if (prefix != "r" && source[pos] == '\\') {
            advance(); // Skip backslash
            if (pos < source.length()) {
                advance(); // Skip the escaped character
            }
        } else {
            advance();
        }
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
