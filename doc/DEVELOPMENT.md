# Vanction 编程语言 - 开发文档

## 1. 项目概述

Vanction 是一种简单的编译型编程语言，支持编译为可执行文件（通过 GCC）或直接解释执行。

## 2. 目录结构

```
VanctionProgramLanguage/
├── include/           # 头文件
│   ├── ast.h          # AST 节点定义
│   └── token.h        # 标记类型定义
├── src/               # 源代码
│   ├── code_generator.cpp  # C++ 代码生成器
│   ├── code_generator.h    # 代码生成器头文件
│   ├── lexer.cpp      # 词法分析器
│   ├── lexer.h        # 词法分析器头文件
│   ├── main.cpp       # 主程序入口
│   ├── parser.cpp     # 语法分析器
│   └── parser.h       # 语法分析器头文件
├── examples/          # 示例程序
│   ├── hello_world.vn # Hello World 示例
│   └── variable_test.vn # 变量和输入输出示例
├── build/             # 构建目录
├── CMakeLists.txt     # CMake 构建配置
├── DEVELOPMENT.md     # 开发文档
└── README.md          # 使用文档
```

## 3. 核心组件

### 3.1 词法分析器（Lexer）

- **功能**：将源代码转换为标记流
- **实现**：`src/lexer.cpp` 和 `src/lexer.h`
- **关键方法**：
  - `getNextToken()`：获取下一个标记
  - `parseIdentifierOrKeyword()`：解析标识符或关键字
  - `parseStringLiteral()`：解析字符串字面量
  - `parseCharLiteral()`：解析字符字面量
  - `parseIntegerLiteral()`：解析整数字面量

### 3.2 语法分析器（Parser）

- **功能**：将标记流转换为抽象语法树（AST）
- **实现**：`src/parser.cpp` 和 `src/parser.h`
- **关键方法**：
  - `parseProgramAST()`：生成整个程序的 AST
  - `parseFunctionAST()`：解析函数定义
  - `parseStatement()`：解析语句
  - `parseExpression()`：解析表达式
  - `parseVariableDeclaration()`：解析变量声明

### 3.3 抽象语法树（AST）

- **功能**：表示程序的结构化语法
- **实现**：`include/ast.h`
- **主要节点类型**：
  - `Program`：程序根节点
  - `FunctionDeclaration`：函数声明
  - `VariableDeclaration`：变量声明
  - `ExpressionStatement`：表达式语句
  - `FunctionCall`：函数调用
  - `BinaryExpression`：二元表达式（如字符串连接）
  - `AssignmentExpression`：赋值表达式
  - 各种字面量节点：`IntegerLiteral`, `CharLiteral`, `StringLiteral`, `BooleanLiteral`

### 3.4 代码生成器（CodeGenerator）

- **功能**：将 AST 转换为 C++ 代码
- **实现**：`src/code_generator.cpp` 和 `src/code_generator.h`
- **关键方法**：
  - `generate()`：生成整个程序的 C++ 代码
  - `generateFunctionDeclaration()`：生成函数声明的 C++ 代码
  - `generateVariableDeclaration()`：生成变量声明的 C++ 代码
  - `generateExpression()`：生成表达式的 C++ 代码

### 3.5 解释器

- **功能**：直接执行 AST
- **实现**：`src/main.cpp` 中的执行函数
- **关键函数**：
  - `executeProgram()`：执行整个程序
  - `executeFunctionDeclaration()`：执行函数声明
  - `executeStatement()`：执行语句
  - `executeExpression()`：执行表达式
  - `executeFunctionCall()`：执行函数调用

## 4. 编译和构建流程

### 4.1 构建要求

- CMake 3.10 或更高版本
- C++ 编译器支持 C++17
- GCC 编译器（用于生成可执行文件）

### 4.2 构建步骤

```powershell
mkdir -p build
cd build
cmake ..
cmake --build .
```

### 4.3 编译流程

1. 源代码 → 词法分析器 → 标记流
2. 标记流 → 语法分析器 → AST
3. AST → 代码生成器 → C++ 代码
4. C++ 代码 → GCC → 可执行文件

### 4.4 解释流程

1. 源代码 → 词法分析器 → 标记流
2. 标记流 → 语法分析器 → AST
3. AST → 解释器 → 直接执行

## 5. 扩展指南

### 5.1 添加新数据类型

1. 在 `include/token.h` 中添加新的标记类型
2. 在 `src/lexer.cpp` 中添加相应的解析方法
3. 在 `include/ast.h` 中添加新的字面量节点类型
4. 在 `src/parser.cpp` 中添加相应的解析方法
5. 在 `src/main.cpp` 中更新 `executeExpression()` 函数
6. 在 `src/code_generator.cpp` 中更新 `generateExpression()` 函数

### 5.2 添加新语法结构

1. 在 `include/ast.h` 中添加新的 AST 节点类型
2. 在 `src/parser.cpp` 中添加相应的解析方法
3. 在 `src/main.cpp` 中添加相应的执行方法
4. 在 `src/code_generator.cpp` 中添加相应的代码生成方法

### 5.3 添加新的系统函数

1. 在 `src/parser.cpp` 中确保能正确解析函数调用
2. 在 `src/main.cpp` 中更新 `executeFunctionCall()` 函数
3. 在 `src/code_generator.cpp` 中更新 `generateFunctionCall()` 函数

## 6. 代码规范

- 所有代码注释使用英文
- 类名使用 PascalCase
- 函数名和变量名使用 camelCase
- 常量使用全大写加下划线
- 每个源文件对应一个头文件
- 头文件使用 `#ifndef` 防护

## 7. 调试技巧

- 使用 `std::cout` 输出中间结果
- 检查生成的 AST 结构
- 查看生成的 C++ 代码
- 使用调试器逐步执行

## 8. 测试

- 在 `examples/` 目录下添加测试用例
- 使用两种运行模式（解释器和编译器）测试
- 测试边界情况和错误情况

## 9. 未来发展方向

- 支持更多数据类型（如浮点数）
- 支持控制流语句（if-else, for, while）
- 支持自定义函数
- 支持数组和结构体
- 支持模块系统
- 优化错误提示
- 添加标准库
