# Vanction Programming Language - Development Documentation

## 1. Project Overview

Vanction is a simple compiled programming language that supports compiling to executable files (via GCC) or direct interpretation.

## 2. .

## 3. Core Components

### 3.1 Lexical Analyzer (Lexer)

- **Function**: Converts source code into a token stream
- **Implementation**: `src/lexer.cpp` and `src/lexer.h`
- **Key Methods**:
  - `getNextToken()`: Get the next token
  - `parseIdentifierOrKeyword()`: Parse identifier or keyword
  - `parseStringLiteral()`: Parse string literal
  - `parseCharLiteral()`: Parse character literal
  - `parseNumberLiteral()`: Parse number literal (integer, float, double)

### 3.2 Syntax Analyzer (Parser)

- **Function**: Converts token stream into Abstract Syntax Tree (AST)
- **Implementation**: `src/parser.cpp` and `src/parser.h`
- **Key Methods**:
  - `parseProgramAST()`: Generate AST for the entire program
  - `parseFunctionAST()`: Parse function definition
  - `parseStatement()`: Parse statement
  - `parseExpression()`: Parse expression
  - `parseVariableDeclaration()`: Parse variable declaration
  - `parsePrimaryExpression()`: Parse primary expression
  - `parseStringLiteral()`: Parse string literal
  - `parseIntegerLiteral()`: Parse integer literal
  - `parseFloatLiteral()`: Parse float literal
  - `parseDoubleLiteral()`: Parse double literal
  - `parseCharLiteral()`: Parse character literal
  - `parseBooleanLiteral()`: Parse boolean literal
  - `parseAssignmentExpression()`: Parse assignment expression
  - `parseBinaryExpression()`: Parse binary expression
  - `parseMultiplicativeExpression()`: Parse multiplicative expression
  - `parseAdditiveExpression()`: Parse additive expression
  - `parseBitShiftExpression()`: Parse bit shift expression
  - `parseRelationalExpression()`: Parse relational expression (comparison operators)
  - `parseLogicalExpression()`: Parse logical expression
  - `parseIfStatement()`: Parse if-else-if-else statement
  - `parseForLoopStatement()`: Parse traditional for loop
  - `parseForInLoopStatement()`: Parse for-in loop
  - `parseWhileLoopStatement()`: Parse while loop
  - `parseDoWhileLoopStatement()`: Parse do-while loop
  - `parseSwitchStatement()`: Parse switch statement
  - `parseReturnStatement()`: Parse return statement

### 3.3 Abstract Syntax Tree (AST)

- **Function**: Represents the structured syntax of the program
- **Implementation**: `include/ast.h`
- **Main Node Types**:
  - `Program`: Root node of the program
  - `FunctionDeclaration`: Function declaration
  - `VariableDeclaration`: Variable declaration
  - `ExpressionStatement`: Expression statement
  - `FunctionCall`: Function call
  - `BinaryExpression`: Binary expression (supports +, -, *, /, <<, >>, ==, !=, <, <=, >, >=, AND, OR, XOR)
  - `AssignmentExpression`: Assignment expression
  - `IfStatement`: if-else-if-else statement
  - `ForLoopStatement`: Traditional for loop
  - `ForInLoopStatement`: for-in loop
  - `WhileLoopStatement`: while loop
  - `DoWhileLoopStatement`: do-while loop
  - `SwitchStatement`: switch statement
  - `CaseStatement`: case label in switch statement
  - `Comment`: Comment node
  - `ReturnStatement`: return statement
  - Various literal nodes: `IntegerLiteral`, `CharLiteral`, `StringLiteral`, `BooleanLiteral`, `FloatLiteral`, `DoubleLiteral`

### 3.4 Code Generator (CodeGenerator)

- **Function**: Converts AST to C++ code
- **Implementation**: `src/code_generator.cpp` and `src/code_generator.h`
- **Key Methods**:
  - `generate()`: Generate C++ code for the entire program
  - `generateFunctionDeclaration()`: Generate C++ code for function declaration
  - `generateVariableDeclaration()`: Generate C++ code for variable declaration
  - `generateStatement()`: Generate C++ code for statement
  - `generateExpression()`: Generate C++ code for expression
  - `generateIdentifier()`: Generate C++ code for identifier
  - `generateIntegerLiteral()`: Generate C++ code for integer literal
  - `generateFloatLiteral()`: Generate C++ code for float literal
  - `generateDoubleLiteral()`: Generate C++ code for double literal
  - `generateCharLiteral()`: Generate C++ code for character literal
  - `generateBooleanLiteral()`: Generate C++ code for boolean literal
  - `generateStringLiteral()`: Generate C++ code for string literal
  - `generateBinaryExpression()`: Generate C++ code for binary expression
  - `generateAssignmentExpression()`: Generate C++ code for assignment expression
  - `generateFunctionCall()`: Generate C++ code for function call
  - `generateIfStatement()`: Generate C++ code for if-else-if-else statement
  - `generateForLoopStatement()`: Generate C++ code for traditional for loop
  - `generateForInLoopStatement()`: Generate C++ code for for-in loop
  - `generateWhileLoopStatement()`: Generate C++ code for while loop
  - `generateDoWhileLoopStatement()`: Generate C++ code for do-while loop
  - `generateSwitchStatement()`: Generate C++ code for switch statement
  - `generateReturnStatement()`: Generate C++ code for return statement

### 3.5 Interpreter

- **Function**: Directly execute AST
- **Implementation**: Execution functions in `src/main.cpp`
- **Key Functions**:
  - `executeProgram()`: Execute the entire program
  - `executeFunctionDeclaration()`: Execute function declaration
  - `executeStatement()`: Execute statement, including return statements
  - `executeExpression()`: Execute expression, supporting multiple operators and data types
  - `executeFunctionCall()`: Execute function call, including type conversion functions
  
- **Supported Data Types**:
  - int
  - float
  - double
  - char
  - string
  - bool
  - undefined
  
- **Supported Operators**:
  - Arithmetic operators: +, -, *, /
  - Bitwise operators: <<, >>
  - Comparison operators: ==, !=, <, <=, >, >=
  - Logical operators: AND, OR, XOR
  - Assignment operator: =

## 4. Compilation and Build Process

### 4.1 Build Requirements

- CMake 3.10 or higher
- C++ compiler supporting C++17
- GCC compiler (for generating executable files)

### 4.2 Command Line Parameters

- `-i`: Interpret the Vanction program
- `-g`: Compile to executable file (using GCC)
- `-o <file>`: Specify output filename for compilation
- `-debug`: Enable debug logging from lexer, parser, and code generator
- `-config <key> [set <value>|get|reset]`: Configure program settings (e.g., -config GCC set <path>)
- `-h, --help`: Show help message

### 4.3 Build Steps

```powershell
mkdir -p build
cd build
cmake ..
cmake --build .
```

### 4.4 Compilation Process

1. Source code → Lexical analyzer → Token stream
2. Token stream → Syntax analyzer → AST
3. AST → Code generator → C++ code
4. C++ code → GCC → Executable file

### 4.5 Interpretation Process

1. Source code → Lexical analyzer → Token stream
2. Token stream → Syntax analyzer → AST
3. AST → Interpreter → Direct execution

## 5. Extension Guide

### 5.1 Adding New Data Types

1. Add new token type in `include/token.h`
2. Update `parseNumberLiteral()` method in `src/lexer.cpp` to support the new number type
3. Add new literal node type in `include/ast.h`
4. Add corresponding parsing method in `src/parser.cpp` (e.g., `parseFloatLiteral()`, `parseDoubleLiteral()`)
5. Update `Value` type and `executeExpression()` function in `src/main.cpp`
6. Update `generateExpression()` function and corresponding literal generation methods in `src/code_generator.cpp`
7. Update `parseStatement()` function in `src/parser.cpp` to support the new data type declaration

### 5.2 Adding New Syntax Structures

1. Add new AST node type in `include/ast.h`
2. Add corresponding parsing method in `src/parser.cpp`
3. Add corresponding execution method in `src/main.cpp`
4. Add corresponding code generation method in `src/code_generator.cpp`

### 5.3 Adding New System Functions

1. Ensure function calls can be parsed correctly in `src/parser.cpp`
2. Update `executeFunctionCall()` function in `src/main.cpp`
3. Update `generateFunctionCall()` function in `src/code_generator.cpp`

## 6. Code Standards

- All code comments should be in English
- Class names use PascalCase
- Function names and variable names use camelCase
- Constants use ALL_CAPS with underscores
- Each source file corresponds to a header file
- Header files use `#ifndef` guards

## 7. Debugging Tips

- Use `std::cout` to output intermediate results
- Check the generated AST structure
- View the generated C++ code
- Use a debugger to step through execution

## 8. Testing

- Add test cases in the `examples/` directory
- Test using both running modes (interpreter and compiler)
- Test boundary cases and error conditions

## 9. Future Development Directions

- Support arrays and structures
- Support module system
- Add standard library
- Support more operators and expressions
- Support object-oriented programming features

## 10. Recently Implemented Features

- **Return Statement Support**: Added support for `return` statements with optional expressions. In the `main` function, the return value is used as the program's exit code when running in interpreter mode.
- **Custom Functions**: Implemented support for defining and calling custom functions
- **Nested Functions**: Added support for defining functions within other functions
- **Improved Error Handling**: Enhanced error messages with more accurate error types and contexts
- **Debug Mode**: Added `-debug` parameter to enable detailed logging from lexer, parser, and code generator
- **Auto Return Type**: Functions can now be defined without explicit return types (default to `auto`)
- **Exit Code Support**: Main function return values are now properly handled as program exit codes in interpreter mode