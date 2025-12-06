#ifndef VANCTION_LEXER_H
#define VANCTION_LEXER_H

#include "../include/token.h"
#include <string>

// Lexer class
class Lexer {
public:
    // Constructor
    Lexer(const std::string& source);
    
    // Get next token
    Token getNextToken();
    
private:
    std::string source;
    size_t pos;
    int line;
    int column;
    
    // Advance one character
    void advance();
    
    // Skip whitespace characters
    void skipWhitespace();
    
    // Parse identifier or keyword
    Token parseIdentifierOrKeyword();
    
    // Parse comment
    Token parseComment();
    
    // Parse string literal
    Token parseStringLiteral();
    
    // Parse character literal
    Token parseCharLiteral();
    
    // Parse number literal (integer, float, or double)
    Token parseNumberLiteral();
};

#endif // VANCTION_LEXER_H
