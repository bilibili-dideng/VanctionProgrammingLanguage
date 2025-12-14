# Vanction 编程语言 - 完整文档

## 1. 简介

Vanction 是一种简单的编译型编程语言，支持编译为可执行文件（通过内置 GCC）或直接解释执行。它设计简洁，语法直观，适合学习和快速开发小型应用程序。

## 2. 安装

### 2.1 从源码构建

|| 确保安装了 CMake 3.10 或更高版本
|| 确保安装了支持 C++17 的编译器

```powershell
mkdir -p build
cd build
cmake ..
cmake --build .
```

构建完成后，可执行文件 `vanction.exe` 将生成在 `build` 目录中。

### 2.2 直接使用

如果你已经有了编译好的 `vanction.exe` 文件，直接将其放在项目根目录或添加到系统 PATH 即可使用。

## 3. 快速入门

### 3.1 Hello World 示例

创建一个名为 `hello_world.vn` 的文件，内容如下：

```vanction
func main() {
    System.print("Hello, Vanction!");
    return 0;
}
```

### 3.2 运行程序

#### 3.2.1 解释器模式

```powershell
vanction.exe -i hello_world.vn
```

#### 3.2.2 编译模式

```powershell
vanction.exe -g hello_world.vn
```

这将生成一个 `hello_world.exe` 文件，然后可以直接运行：

```powershell
hello_world.exe
```

## 4. 语法参考

### 4.1 基本语法

#### 4.1.1 注释

```vanction
|| 这是单行注释
```

#### 4.1.2 主函数

所有程序必须包含一个 `main` 函数作为入口点：

```vanction
func main() {
    || 函数体
    return 0;
}
```

### 4.2 变量

#### 4.2.1 变量声明与初始化

```vanction
|| 带类型的变量声明与初始化
int num = 123;
float f = 3.14;
double d = 2.71828;
char ch = 'a';
string str = "Hello";
bool flag = true;

|| 自动类型推断
auto num = 456;
auto str = "World";
auto f = 3.14;

|| 仅声明不初始化，声明后默认值为undefined
define var;
```

#### 4.2.2 变量赋值

```vanction
num = 789;
str = "New string";
f = 6.28;
```

### 4.3 数据类型

| 类型 | 描述 | 示例 |
|------|------|------|
| int | 整数 | `int num = 123;` |
| float | 单精度浮点数 | `float f = 3.14;` |
| double | 双精度浮点数 | `double d = 2.71828;` |
| char | 字符 | `char ch = 'a';` |
| string | 字符串 | `string str = "Hello";` |
| bool | 布尔值 | `bool flag = true;` |
| List | 列表类型，支持整数元素 | `List list = [1, 2, 3];` |
| HashMap | 哈希表类型，支持字符串键值对 | `HashMap map = {"a"="1", "b"="2"};` |
| auto | 自动类型推断 | `auto x = 456;` |
| undefined | 未定义值 | `define var;` |

### 4.4 运算符

#### 4.4.1 算术运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| + | 加法/字符串拼接 | `a + b` |
| - | 减法 | `a - b` |
| * | 乘法 | `a * b` |
| / | 除法 | `a / b` |

#### 4.4.2 比较运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| == | 等于 | `a == b` |
| != | 不等于 | `a != b` |
| < | 小于 | `a < b` |
| <= | 小于等于 | `a <= b` |
| > | 大于 | `a > b` |
| >= | 大于等于 | `a >= b` |

#### 4.4.3 位运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| << | 左移 | `a << 1` |
| >> | 右移 | `a >> 1` |

#### 4.4.4 逻辑运算符

| 运算符 | 描述 | 示例 |
|--------|------|------|
| & | 逻辑与 | `a & b` |
| \| | 逻辑或 | `a \| b` |
| ^ | 逻辑异或 | `a ^ b` |

### 4.5 控制流语句

#### 4.5.1 if-else-if-else 语句

```vanction
|| 简单 if 语句
if (条件) {
    || 代码块
}

|| if-else 语句
if (条件) {
    || 代码块
} else {
    || 代码块
}

|| if-else-if-else 语句
if (条件1) {
    || 代码块1
} else-if (条件2) {
    || 代码块2
} else {
    || 代码块3
}
```

#### 4.5.2 for 循环

```vanction
|| 传统 for 循环
for (初始化; 条件; 递增) {
    || 代码块
}

|| 示例
for (i = 1; i <= 3; i = i + 1) {
    System.print(i);
}
```

#### 4.5.3 for-in 循环

```vanction
|| 用于字符串迭代的 for-in 循环
for (char c in 字符串) {
    || 代码块
}

|| 用于列表迭代的 for-in 循环
for (int i in 列表) {
    || 代码块
}

|| 用于哈希表迭代的 for-in 循环
for (key, value in 哈希表) {
    || 代码块
}
```

#### 4.5.4 while 循环

```vanction
|| while 循环
while (条件) {
    || 代码块
}

|| 示例
auto i = 1;
while (i <= 3) {
    System.print(i);
    i = i + 1;
}
```

#### 4.5.5 do-while 循环

```vanction
|| do-while 循环
|| 保证至少执行一次
do {
    || 代码块
} while(条件)

|| 示例
auto i = 1;
do {
    System.print(i);
    i = i + 1;
} while(i <= 3)
```

### 4.6 类型转换

```vanction
|| 将其他类型转换为整数
int a = type.int(b);

|| 将其他类型转换为浮点数
float f = type.float(b);

|| 将其他类型转换为双精度浮点数
double d = type.double(b);

|| 将其他类型转换为字符串
string s = type.string(b);
```

### 4.7 输入输出

#### 4.7.1 输出

```vanction
|| 输出字符串
System.print("Hello, Vanction!");

|| 输出变量
int num = 123;
System.print(num);

|| 字符串连接
string name = "Vanction";
System.print("Hello, " + name + "!");
```

#### 4.7.2 输入

```vanction
|| 获取用户输入，带提示信息
string name = System.input("请输入你的名字：");
System.print("你好，" + name + "!");

|| 获取用户输入，不带提示信息
string input = System.input("");
```

## 5. 核心组件

### 5.1 词法分析器（Lexer）

|| 功能：将源代码转换为标记流
|| 实现：`src/lexer.cpp` 和 `src/lexer.h`
|| 关键方法：
||   - `getNextToken()`：获取下一个标记
||   - `parseIdentifierOrKeyword()`：解析标识符或关键字
||   - `parseStringLiteral()`：解析字符串字面量
||   - `parseCharLiteral()`：解析字符字面量
||   - `parseNumberLiteral()`：解析数字字面量（整数、浮点数、双精度浮点数）

### 5.2 语法分析器（Parser）

|| 功能：将标记流转换为抽象语法树（AST）
|| 实现：`src/parser.cpp` 和 `src/parser.h`
|| 关键方法：
||   - `parseProgramAST()`：生成整个程序的 AST
||   - `parseFunctionAST()`：解析函数定义
||   - `parseStatement()`：解析语句
||   - `parseExpression()`：解析表达式

### 5.3 代码生成器（CodeGenerator）

|| 功能：将 AST 转换为 C++ 代码
|| 实现：`src/code_generator.cpp` 和 `src/code_generator.h`
|| 关键方法：
||   - `generate()`：生成整个程序的 C++ 代码
||   - `generateFunctionDeclaration()`：生成函数声明的 C++ 代码
||   - `generateStatement()`：生成语句的 C++ 代码
||   - `generateExpression()`：生成表达式的 C++ 代码

### 5.4 解释器

|| 功能：直接执行 AST
|| 实现：`src/main.cpp` 中的执行函数
|| 关键函数：
||   - `executeProgram()`：执行整个程序
||   - `executeStatement()`：执行语句
||   - `executeExpression()`：执行表达式
||   - `executeFunctionCall()`：执行函数调用

### 5.5 错误处理系统

|| 功能：提供完善的错误处理机制，包括编译时错误检查和运行时错误处理
|| 实现：`src/error.cpp` 和 `src/error.h`
|| 主要组件：
||   - `ErrorType`：错误类型枚举
||   - `vanction_error`：错误命名空间，包含所有具体的错误类
||   - `Error`：表示错误的类，包含错误类型、消息、文件路径、行号和列号

## 6. RRP 标准

### RRP-1-FUNCTION.md（函数标准）

#### 6.1.1 主函数

```Vanction
func main() {
    || 函数体
    return 0; || 可选，返回值最终返回给操作系统
}
```

#### 6.1.2 自定义函数

```Vanction
func function_name(parameter1, parameter2, ...) {
    || 函数体
    return 返回值; || 可选
}

|| 调用示例
function_name(parameter1, parameter2, ...);
```

### RRP-2-DefineVariable.md（变量定义标准）

```Vanction
int variable_name = value; || 整型变量
string variable_name = value; || 字符串变量
List variable_name = value; || 列表变量
HashMap variable_name = value; || 哈希表变量
auto variable_name = value; || 自动类型推断
define variable_name; || 定义未初始化变量，值为undefined

|| 变量使用示例
System.print(variable_name);
```

### RRP-3-System_Class.md（System类标准）

```Vanction
|| System.print 示例
System.print("Hello, Vanction!"); || 输出字符串
System.print(123); || 输出整数
System.print(f"格式化字符串：{变量名}"); || 输出格式化字符串

|| System.input 示例
string input = System.input("请输入："); || 带提示的输入
string input = System.input(""); || 无提示的输入
```

## 7. 面向对象编程

### 7.1 类定义

```vanction
|| 类声明语法
class 类名(父类名) {
    || 实例初始化方法
    instance.init(instance, 参数) {
        || 初始化实例变量
        instance.变量名 = 值;
    }
    
    || 实例方法
    instance.方法名(instance, 参数) {
        || 方法体
        return 结果;
    }
    
    || 类方法
    class.方法名() {
        || 方法体
        return 结果;
    }
}
```

### 7.2 实例创建与使用

```vanction
|| 创建实例
person1 = instance Person("Alice", 30);

|| 调用实例方法
string name = person1.getName();
int age = person1.getAge();

|| 访问实例变量
System.print(person1.name);

|| 修改实例变量
person1.age = 31;
```

## 8. 错误处理

### 8.1 try-happen 错误处理

```vanction
try {
    || 可能抛出错误的代码
} happen (ErrorType) as errorVariable {
    || 处理错误的代码
    System.print(errorVariable.text); || 完整错误信息
    System.print(errorVariable.type); || 错误类型
    System.print(errorVariable.info); || 错误详情
}
```

## 9. 命令行选项

| 选项 | 描述 |
|------|------|
| `-i <file.vn>` | 解释器模式，直接执行程序 |
| `-g <file.vn>` | 编译模式，生成可执行文件 |
| `-o <output.exe>` | 指定编译输出的可执行文件名 |
| `-debug` | 启用调试日志 |
| `-config <key> [set <value>\|\|get\|\|reset]` | 配置程序设置 |
| `-h, --help` | 显示帮助信息 |

## 10. 示例程序

### 10.1 变量和输入输出示例

```vanction
func main() {
    || 测试变量声明与初始化功能
    
    || 整数类型
    int num1 = 123;
    System.print(num1);
    
    || 浮点数类型
    float f = 3.14;
    double d = 2.71828;
    System.print(f);
    System.print(d);
    
    || 字符类型
    char char1 = 'a';
    System.print(char1);
    
    || 字符串类型
    string str1 = "Hello, Vanction!";
    System.print(str1);
    
    || 布尔类型
    bool flag1 = true;
    bool flag2 = false;
    System.print(flag1);
    System.print(flag2);
    
    || 自动类型推断
    auto auto1 = 456;
    System.print(auto1);
    
    auto auto2 = "Auto string";
    System.print(auto2);
    
    || 算术运算
    System.print(10 + 5);
    System.print(10 - 5);
    System.print(10 * 5);
    System.print(10 / 5);
}
```

### 10.2 for-in 循环示例

```vanction
func main() {
    || List 遍历
    List list = [1, 2, 3, 4, 5];
    System.print("List elements:");
    for (i in list) {
        System.print(i);
    }
    
    || HashMap 遍历
    HashMap map = {"a"="1", "b"="2", "c"="3"};
    System.print("HashMap elements:");
    for (key, value in map) {
        System.print("Key: " + key + ", Value: " + value);
    }
    
    || range 遍历
    System.print("Range from 0 to 4:");
    for (i in range(5)) {
        System.print(i);
    }
}
```

## 11. 已知限制

|| 仅支持单行注释
|| 字符串操作仅支持 `+` 拼接运算符
|| List类型仅支持整数元素
|| HashMap类型仅支持字符串键值对

## 12. 未来计划

|| 支持多行注释
|| 添加更多系统函数
|| 支持数组和结构体
|| 支持模块系统
|| 添加标准库

## 13. 项目结构

```
VanctionProgramLanguage/
├── bin/                 || 二进制输出目录
├── CMakeLists.txt       || CMake 配置文件
├── doc/                 || 文档目录
│   ├── DEVELOPMENT.md   || 开发文档 (英文)
│   └── ZH_DEVELOPMENT.md|| 开发文档 (中文)
├── examples/            || 示例程序
├── include/             || 公共头文件
├── src/                 || 源代码
└── README.md            || 项目文档
```

## 14. 许可证

本项目采用 LGPL2.0 许可证。

## 15. 联系方式

|| 项目地址：[VanctionProgramLanguage](https://github.com/bilibili-dideng/VanctionProgrammingLanguage)
|| 邮箱：3483434955@qq.com
