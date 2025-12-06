# Vanction Programming Language

## 1. Introduction

Vanction is a simple compiled programming language that supports compiling to executable files (via built-in GCC) or direct interpretation. It has a clean design and intuitive syntax, making it suitable for learning and rapid development of small applications.

## 2. Installation

### 2.1 Building from Source

1. Ensure CMake 3.10 or higher is installed
2. Ensure a C++ compiler supporting C++17 is installed
3. Clone or download the project source code
4. Execute the following commands to build:

```powershell
mkdir -p build
cd build
cmake ..
cmake --build .
```

After building, the executable file `vanction.exe` will be generated in the `build` directory.

### 2.2 Direct Usage

If you already have a compiled `vanction.exe` file, simply place it in the project root directory or add it to your system PATH to use it.

## 3. Quick Start

### 3.1 Hello World Example

Create a file named `hello_world.vn` with the following content:

```vanction
func main() {
    System.print("Hello, Vanction!");
    return 0;
}
```

The `return` statement in the `main` function will be used as the program's exit code when running in interpreter mode.

### 3.2 Running the Program

#### 3.2.1 Interpreter Mode

```powershell
vanction.exe -i hello_world.vn
```

#### 3.2.2 Compilation Mode

```powershell
vanction.exe -g hello_world.vn
```

This will generate a `hello_world.exe` file, which can then be run directly:

```powershell
hello_world.exe
```

## 4. Syntax Reference

### 4.1 Basic Syntax

#### 4.1.1 Comments

```vanction
| This is a single-line comment
```

#### 4.1.2 Main Function

All programs must contain a `main` function as the entry point:

```vanction
func main() {
    | Function body
    return 0;
}
```

The `main` function can return an integer value, which will be used as the program's exit code when running in interpreter mode. If no return statement is provided, the program will exit with code 0 by default.

### 4.2 Variables

#### 4.2.1 Variable Declaration and Initialization

```vanction
| Type-specific variable declaration and initialization
int num = 123;
float f = 3.14;
double d = 2.71828;
char ch = 'a';
string str = "Hello";
bool flag = true;

| Automatic type inference
auto num = 456;
auto str = "World";
auto f = 3.14;

| Declaration without initialization, default value is undefined
define var;
```

#### 4.2.2 Variable Assignment

```vanction
num = 789;
str = "New string";
f = 6.28;
```

### 4.3 Data Types

| Type | Description | Example |
|------|-------------|---------|
| int | Integer | `int num = 123;` |
| float | Single-precision floating point | `float f = 3.14;` |
| double | Double-precision floating point | `double d = 2.71828;` |
| char | Character | `char ch = 'a';` |
| string | String | `string str = "Hello";` |
| bool | Boolean | `bool flag = true;` |
| auto | Automatic type inference | `auto x = 456;` |
| undefined | Undefined value | `define var;` |

### 4.4 Operators

#### 4.4.1 Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| + | Addition/String concatenation | `a + b` |
| - | Subtraction | `a - b` |
| * | Multiplication | `a * b` |
| / | Division | `a / b` |

#### 4.4.2 Comparison Operators

| Operator | Description | Example |
|----------|-------------|---------|
| == | Equal to | `a == b` |
| != | Not equal to | `a != b` |
| < | Less than | `a < b` |
| <= | Less than or equal to | `a <= b` |
| > | Greater than | `a > b` |
| >= | Greater than or equal to | `a >= b` |

#### 4.4.3 Bitwise Operators

| Operator | Description | Example |
|----------|-------------|---------|
| << | Left shift | `a << 1` |
| >> | Right shift | `a >> 1` |

#### 4.4.4 Logical Operators

| Operator | Description | Example |
|----------|-------------|---------|
| AND | Logical AND | `a AND b` |
| OR | Logical OR | `a OR b` |
| XOR | Logical XOR | `a XOR b` |

### 4.5 Control Flow Statements

#### 4.5.1 if-else-if-else Statement

```vanction
| Simple if statement
if (condition) {
    | Code block
}

| if-else statement
if (condition) {
    | Code block
} else {
    | Code block
}

| if-else-if-else statement
if (condition1) {
    | Code block 1
} else-if (condition2) {
    | Code block 2
} else {
    | Code block 3
}
```

#### 4.5.2 for Loop

```vanction
| Traditional for loop
for (initialization; condition; increment) {
    | Code block
}

| Example
for (i = 1; i <= 3; i = i + 1) {
    System.print(i);
}
```

#### 4.5.3 for-in Loop

```vanction
| for-in loop for string iteration
for (char c in string) {
    | Code block
}

| Example
string s = "abc";
for (char c in s) {
    System.print(c);
}
```

#### 4.5.4 while Loop

```vanction
| while loop
while (condition) {
    | Code block
}

| Example
auto i = 1;
while (i <= 3) {
    System.print(i);
    i = i + 1;
}
```

#### 4.5.5 do-while Loop

```vanction
| do-while loop
| Guaranteed to execute at least once
do {
    | Code block
} while(condition)

| Example
auto i = 1;
do {
    System.print(i);
    i = i + 1;
} while(i <= 3)
```

#### 4.5.6 switch Statement

```vanction
| switch statement
switch(expression) {
    case value1 {
        | Code block 1
    }
    case value2 {
        | Code block 2
    }
    // More cases...
}

| Example
auto num = 2;
switch(num) {
    case 1 {
        System.print("One");
    }
    case 2 {
        System.print("Two");
    }
    case 3 {
        System.print("Three");
    }
}
```

#### 4.5.7 return Statement

The `return` statement is used to exit a function and optionally return a value. In the `main` function, the return value is used as the program's exit code in interpreter mode.

```vanction
| return statement without expression
return;

| return statement with expression
return expression;

| Example in main function
func main() {
    System.print("Hello, Vanction!");
    return 0;  // Exit with code 0
}
```

### 4.6 Type Conversion

Vanction provides type conversion functions for converting between different data types:

```vanction
| Convert other types to integer
int a = type.int(b);

| Convert other types to float
float f = type.float(b);

| Convert other types to double
double d = type.double(b);

| Convert other types to string
string s = type.string(b);
```

### 4.7 Input/Output

#### 4.7.1 Output

```vanction
| Output string
System.print("Hello, Vanction!");

| Output variable
int num = 123;
System.print(num);

| String concatenation
string name = "Vanction";
System.print("Hello, " + name + "!");
```

#### 4.7.2 Input

```vanction
| Get user input with prompt
string name = System.input("Please enter your name: ");
System.print("Hello, " + name + "!");

| Get user input without prompt
string input = System.input("");
```

### 4.8 Namespaces

Namespaces are used to organize code and avoid name collisions. They allow you to group related functions together under a common name.

#### 4.8.1 Namespace Declaration

```vanction
| Namespace declaration syntax
namespace NamespaceName {
    | Functions and other declarations go here
}

| Example
namespace Math {
    func add(a, b) {
        return a + b;
    }
    
    func multiply(a, b) {
        return a * b;
    }
}
```

#### 4.8.2 Accessing Namespace Members

To access a function or variable within a namespace, use the colon operator `:`:

```vanction
| Accessing namespace members
result = NamespaceName:memberName;

| Example
func main() {
    int sum = Math:add(3, 4);
    int product = Math:multiply(3, 4);
    System.print("Sum: ", sum);
    System.print("Product: ", product);
}
```

## 5. Error Handling

Vanction has a comprehensive error handling system that provides clear and formatted error messages. When an error occurs, the interpreter/compiler will display:

- A blue header "error occurred to:"
- The purple absolute file path
- Three purple lines of code around the error
- A red error indicator `^^^^^^^` pointing to the error
- The purple error type and message

### 5.1 Error Types

| Error Type | Description |
|------------|-------------|
| `CError` | C++ errors during compilation or execution |
| `CompilationError` | Errors from the external compiler (GCC) |
| `DivideByZeroError` | Division by zero errors |
| `ValueError` | Type conversion errors (e.g., converting string "abc" to int) |
| `TokenError` | Unknown token errors |
| `SyntaxError` | Syntax errors |
| `MainFunctionError` | Missing main function |

### 5.2 Error Message Example

```
error occurred to:
    C:\Users\Administrator\CLionProjects\VanctionProgramLanguage\test_error.vn
    func main() {
        int a = 10 / 0;
    ^^^^^^^
DivideByZeroError: Division by zero
```

## 6. Command Line Options

| Option | Description |
|--------|-------------|
| `-i <file.vn>` | Interpreter mode, execute the program directly |
| `-g <file.vn>` | Compilation mode, generate executable file |
| `-o <output.exe>` | Specify output executable filename for compilation |
| `-debug` | Enable debug logging for lexer, parser, main, and codegenerator |
| `-config <key> [set <value>|get|reset]` | Configure program settings (e.g., -config GCC set <path>) |
| `-h, --help` | Show help information |

### 6.1 Examples

```powershell
// Interpret execution
vanction.exe -i examples/hello_world.vn

// Compile to executable
vanction.exe -g examples/hello_world.vn

// Compile and specify output filename
vanction.exe -g examples/hello_world.vn -o hello.exe

// Get GCC path configuration
vanction.exe -config GCC get

// Set GCC path configuration
vanction.exe -config GCC set "path/to/g++.exe"

// Reset GCC path to default
vanction.exe -config GCC reset
```

## 7. Sample Programs

### 7.1 Variable and Input/Output Example

```vanction
func main() {
    | Test variable declaration and initialization
    
    | Integer type
    int num1 = 123;
    System.print(num1);
    
    | Floating point types
    float f = 3.14;
    double d = 2.71828;
    System.print(f);
    System.print(d);
    
    | Character type
    char char1 = 'a';
    System.print(char1);
    
    | String type
    string str1 = "Hello, Vanction!";
    System.print(str1);
    
    | Boolean type
    bool flag1 = true;
    bool flag2 = false;
    System.print(flag1);
    System.print(flag2);
    
    | Automatic type inference
    auto auto1 = 456;
    System.print(auto1);
    
    auto auto2 = "Auto string";
    System.print(auto2);
    
    | Arithmetic operations
    System.print(10 + 5);
    System.print(10 - 5);
    System.print(10 * 5);
    System.print(10 / 5);
    
    | Bitwise operations
    System.print(8 << 1);
    System.print(8 >> 1);
    
    | Logical operations
    System.print(flag1 AND flag2);
    System.print(flag1 OR flag2);
    System.print(flag1 XOR flag2);
    
    | Type conversion
    int i = 42;
    float f2 = type.float(i);
    System.print(f2);
    
    | Input function test
    string input1 = System.input("Please enter your name: ");
    System.print("Hello, " + input1 + "!");
}
```

## 8. Notes

1. All statements must end with a semicolon `;`
2. String literals are enclosed in double quotes `""`
3. Character literals are enclosed in single quotes `''`
4. Boolean values are `true` or `false` (lowercase)
5. Compilation mode requires built-in GCC support, ensure the `mingw64` directory is in the same directory as `vanction.exe`
6. Variable names can only contain letters and numbers, and cannot start with a number

## 9. Known Limitations

1. Does not support arrays and structures
2. Only supports single-line comments
3. String operations only support `+` concatenation operator
4. for-in loops only support string iteration

## 10. Future Plans

- Support arrays and structures
- Support multi-line comments
- Add more system functions
- Expand for-in loop support to other collection types

## 11. Project Structure

```
VanctionProgramLanguage/
├── bin/                 | Binary output directory
├── CMakeLists.txt       | CMake configuration file
├── doc/                 | Documentation directory
│   ├── DEVELOPMENT.md   | Development documentation (English)
│   └── ZH_DEVELOPMENT.md| Development documentation (Chinese)
├── examples/            | Example programs
│   ├── hello_world.vn   | Hello World example
│   └── namespace_test.vn | Namespace functionality example
├── include/             | Public header files
│   ├── ast.h            | Abstract Syntax Tree definitions
│   └── token.h          | Token definitions
├── LICENSE              | LGPL2.0 license file
├── README.md            | Project documentation (English)
├── src/                 | Source code
│   ├── code_generator.cpp | Code generation implementation
│   ├── code_generator.h   | Code generation header
│   ├── error.cpp          | Error handling implementation
│   ├── error.h            | Error handling header
│   ├── lexer.cpp          | Lexical analyzer implementation
│   ├── lexer.h            | Lexical analyzer header
│   ├── main.cpp           | Main program entry
│   ├── parser.cpp         | Parser implementation
│   └── parser.h           | Parser header
└── ZH_README.md         | Project documentation (Chinese)
```

**Important Note:** Compilation mode requires mingw64 to be installed. Ensure the `mingw64` directory is in the same directory as `vanction.exe` or properly configured in your system PATH.

## 12. License

This project is licensed under the LGPL2.0 License.

## 13. Contributions

Issues and Pull Requests are welcome!

## 14. Contact

For questions or suggestions, please contact:

- Project Address: [VanctionProgramLanguage](https://github.com/bilibili-dideng/VanctionProgrammingLanguage)
- Email: 3483434955@qq.com