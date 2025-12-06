#ifndef VANCTION_TOKEN_H
#define VANCTION_TOKEN_H

#include <string>

// Token type enumeration
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    STRING_LITERAL,
    CHAR_LITERAL,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    DOUBLE_LITERAL,
    DOT,
    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    ASSIGN,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LSHIFT,
    RSHIFT,
    AND,
    OR,
    XOR,
    COMMENT,
    EOF_TOKEN,
    // Comparison operators
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    // Control flow keywords (will be handled as KEYWORD type with specific values)
}; 

// Token structure
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

#endif // VANCTION_TOKEN_H
