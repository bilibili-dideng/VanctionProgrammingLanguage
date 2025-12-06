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
}
```

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
}
```

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

#### 4.4.2 Bitwise Operators

| Operator | Description | Example |
|----------|-------------|---------|
| << | Left shift | `a << 1` |
| >> | Right shift | `a >> 1` |

#### 4.4.3 Logical Operators

| Operator | Description | Example |
|----------|-------------|---------|
| AND | Logical AND | `a AND b` |
| OR | Logical OR | `a OR b` |
| XOR | Logical XOR | `a XOR b` |

### 4.5 Type Conversion

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

### 4.6 Input/Output

#### 4.6.1 Output

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

#### 4.6.2 Input

```vanction
| Get user input with prompt
string name = System.input("Please enter your name: ");
System.print("Hello, " + name + "!");

| Get user input without prompt
string input = System.input("");
```

## 5. Command Line Options

| Option | Description |
|--------|-------------|
| `-i <file.vn>` | Interpreter mode, execute the program directly |
| `-g <file.vn>` | Compilation mode, generate executable file |
| `-o <output.exe>` | Specify output executable filename for compilation |
| `-h, --help` | Show help information |

### 5.1 Examples

```powershell
// Interpret execution
vanction.exe -i examples/hello_world.vn

// Compile to executable
vanction.exe -g examples/hello_world.vn

// Compile and specify output filename
vanction.exe -g examples/hello_world.vn -o hello.exe
```

## 6. Sample Programs

### 6.1 Variable and Input/Output Example

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

## 7. Notes

1. All statements must end with a semicolon `;`
2. String literals are enclosed in double quotes `""`
3. Character literals are enclosed in single quotes `''`
4. Boolean values are `true` or `false` (lowercase)
5. Compilation mode requires built-in GCC support, ensure the `mingw64` directory is in the same directory as `vanction.exe`
6. Variable names can only contain letters and numbers, and cannot start with a number

## 8. Known Limitations

1. Does not support control flow statements (if-else, for, while)
2. Does not support custom functions
3. Does not support arrays and structures
4. Only supports single-line comments
5. String operations only support `+` concatenation operator

## 9. Future Plans

- Support control flow statements (if-else, for, while)
- Support custom functions
- Support arrays and structures
- Support multi-line comments
- Add more system functions
- Optimize error messages

## 10. Project Structure

```
VanctionProgramLanguage/
├── .idea/              | IDE configuration files
├── build/              | Build output directory
├── doc/                | Documentation directory
├── examples/           | Example programs
├── include/            | Header files
├── mingw64/            | GCC compiler
├── src/                | Source code
├── .gitignore          | Git ignore configuration
├── CMakeLists.txt      | CMake configuration
└── README.md           | Project documentation
```

## 11. License

This project is licensed under the MIT License.

## 12. Contributions

Issues and Pull Requests are welcome!

## 13. Contact

For questions or suggestions, please contact:

- Project Address: [VanctionProgramLanguage](https://github.com/bilibili-dideng/VanctionProgramLanguage)
- Email: 3483434955@qq.com