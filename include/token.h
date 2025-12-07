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
    COLON,
    SEMICOLON,
    COMMA,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    ASSIGN,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    LSHIFT,
    RSHIFT,
    AND,
    OR,
    XOR,
    NOT,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_NOT,
    COMMENT,
    EOF_TOKEN,
    // Comparison operators
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    // Compound assignment operators
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    MULTIPLY_ASSIGN,
    DIVIDE_ASSIGN,
    MODULO_ASSIGN,
    LSHIFT_ASSIGN,
    RSHIFT_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    // Increment/decrement operators
    INCREMENT,
    DECREMENT,
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
