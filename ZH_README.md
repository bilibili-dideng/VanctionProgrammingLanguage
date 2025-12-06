# Vanction 编程语言

## 1. 简介

Vanction 是一种简单的编译型编程语言，支持编译为可执行文件（通过内置 GCC）或直接解释执行。它设计简洁，语法直观，适合学习和快速开发小型应用程序。

## 2. 安装

### 2.1 从源码构建

1. 确保安装了 CMake 3.10 或更高版本
2. 确保安装了支持 C++17 的编译器
3. 克隆或下载项目源码
4. 执行以下命令构建：

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
| 这是单行注释
```

#### 4.1.2 主函数

所有程序必须包含一个 `main` 函数作为入口点：

```vanction
func main() {
    | 函数体
}
```

### 4.2 变量

#### 4.2.1 变量声明与初始化

```vanction
| 带类型的变量声明与初始化
int num = 123;
float f = 3.14;
double d = 2.71828;
char ch = 'a';
string str = "Hello";
bool flag = true;

| 自动类型推断
auto num = 456;
auto str = "World";
auto f = 3.14;

| 仅声明不初始化，声明后默认值为undefined
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
| AND | 逻辑与 | `a AND b` |
| OR | 逻辑或 | `a OR b` |
| XOR | 逻辑异或 | `a XOR b` |

### 4.5 控制流语句

#### 4.5.1 if-else-if-else 语句

```vanction
| 简单 if 语句
if (条件) {
    | 代码块
}

| if-else 语句
if (条件) {
    | 代码块
} else {
    | 代码块
}

| if-else-if-else 语句
if (条件1) {
    | 代码块1
} else-if (条件2) {
    | 代码块2
} else {
    | 代码块3
}
```

#### 4.5.2 for 循环

```vanction
| 传统 for 循环
for (初始化; 条件; 递增) {
    | 代码块
}

| 示例
for (i = 1; i <= 3; i = i + 1) {
    System.print(i);
}
```

#### 4.5.3 for-in 循环

```vanction
| 用于字符串迭代的 for-in 循环
for (char c in 字符串) {
    | 代码块
}

| 示例
string s = "abc";
for (char c in s) {
    System.print(c);
}
```

#### 4.5.4 while 循环

```vanction
| while 循环
while (条件) {
    | 代码块
}

| 示例
auto i = 1;
while (i <= 3) {
    System.print(i);
    i = i + 1;
}
```

#### 4.5.5 do-while 循环

```vanction
| do-while 循环
| 保证至少执行一次
do {
    | 代码块
} while(条件)

| 示例
auto i = 1;
do {
    System.print(i);
    i = i + 1;
} while(i <= 3)
```

#### 4.5.6 switch 语句

```vanction
| switch 语句
switch(表达式) {
    case 值1 {
        | 代码块1
    }
    case 值2 {
        | 代码块2
    }
    // 更多 case...
}

| 示例
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

### 4.6 类型转换

Vanction 提供了类型转换函数，用于在不同数据类型之间转换：

```vanction
| 将其他类型转换为整数
int a = type.int(b);

| 将其他类型转换为浮点数
float f = type.float(b);

| 将其他类型转换为双精度浮点数
double d = type.double(b);

| 将其他类型转换为字符串
string s = type.string(b);
```

### 4.7 输入输出

#### 4.7.1 输出

```vanction
| 输出字符串
System.print("Hello, Vanction!");

| 输出变量
int num = 123;
System.print(num);

| 字符串连接
string name = "Vanction";
System.print("Hello, " + name + "!");
```

#### 4.7.2 输入

```vanction
| 获取用户输入，带提示信息
string name = System.input("请输入你的名字：");
System.print("你好，" + name + "!");

| 获取用户输入，不带提示信息
string input = System.input("");
```

## 5. 错误处理

Vanction 拥有完善的错误处理系统，提供清晰格式化的错误信息。当错误发生时，解释器/编译器将显示：

- 蓝色标题 "error occurred to:"
- 紫色的绝对文件路径
- 错误周围三行紫色的代码
- 红色的错误指示器 `^^^^^^^` 指向错误位置
- 紫色的错误类型和错误信息

### 5.1 错误类型

| 错误类型 | 描述 |
|----------|------|
| `CError` | 编译或执行过程中的 C++ 错误 |
| `CompilationError` | 外部编译器 (GCC) 错误 |
| `DivideByZeroError` | 除零错误 |
| `ValueError` | 类型转换错误（例如将字符串 "abc" 转换为整数） |
| `TokenError` | 未知标记错误 |
| `SyntaxError` | 语法错误 |
| `MainFunctionError` | 缺少主函数 |

### 5.2 错误信息示例

```
error occurred to:
    C:\Users\Administrator\CLionProjects\VanctionProgramLanguage\test_error.vn
    func main() {
        int a = 10 / 0;
    ^^^^^^^
DivideByZeroError: Division by zero
```

## 6. 命令行选项

| 选项 | 描述 |
|------|------|
| `-i <file.vn>` | 解释器模式，直接执行程序 |
| `-g <file.vn>` | 编译模式，生成可执行文件 |
| `-o <output.exe>` | 指定编译输出的可执行文件名 |
| `-h, --help` | 显示帮助信息 |

### 6.1 示例

```powershell
// 解释执行
vanction.exe -i examples/hello_world.vn

// 编译为可执行文件
vanction.exe -g examples/hello_world.vn

// 编译并指定输出文件名
vanction.exe -g examples/hello_world.vn -o hello.exe
```

## 7. 示例程序

### 7.1 变量和输入输出示例

```vanction
func main() {
    | 测试变量声明与初始化功能
    
    | 整数类型
    int num1 = 123;
    System.print(num1);
    
    | 浮点数类型
    float f = 3.14;
    double d = 2.71828;
    System.print(f);
    System.print(d);
    
    | 字符类型
    char char1 = 'a';
    System.print(char1);
    
    | 字符串类型
    string str1 = "Hello, Vanction!";
    System.print(str1);
    
    | 布尔类型
    bool flag1 = true;
    bool flag2 = false;
    System.print(flag1);
    System.print(flag2);
    
    | 自动类型推断
    auto auto1 = 456;
    System.print(auto1);
    
    auto auto2 = "Auto string";
    System.print(auto2);
    
    | 算术运算
    System.print(10 + 5);
    System.print(10 - 5);
    System.print(10 * 5);
    System.print(10 / 5);
    
    | 位运算
    System.print(8 << 1);
    System.print(8 >> 1);
    
    | 逻辑运算
    System.print(flag1 AND flag2);
    System.print(flag1 OR flag2);
    System.print(flag1 XOR flag2);
    
    | 类型转换
    int i = 42;
    float f2 = type.float(i);
    System.print(f2);
    
    | 输入功能测试
    string input1 = System.input("请输入你的名字：");
    System.print("你好，" + input1 + "!");
}
```

## 8. 注意事项

1. 所有语句必须以分号 `;` 结尾
2. 字符串字面量使用双引号 `""` 包裹
3. 字符字面量使用单引号 `''` 包裹
4. 布尔值为 `true` 或 `false`（小写）
5. 编译模式需要内置的 GCC 支持，确保 `mingw64` 目录与 `vanction.exe` 处于同一目录
6. 变量名只能包含字母和数字，且不能以数字开头

## 9. 已知限制

1. 不支持自定义函数
2. 不支持数组和结构体
3. 仅支持单行注释
4. 字符串操作仅支持 `+` 拼接运算符
5. for-in 循环仅支持字符串迭代

## 10. 未来计划

- 支持自定义函数
- 支持数组和结构体
- 支持多行注释
- 添加更多系统函数
- 扩展 for-in 循环支持到其他集合类型

## 11. 项目结构

```
VanctionProgramLanguage/
├── bin/                 | 二进制输出目录
├── CMakeLists.txt       | CMake 配置文件
├── doc/                 | 文档目录
│   ├── DEVELOPMENT.md   | 开发文档 (英文)
│   └── ZH_DEVELOPMENT.md| 开发文档 (中文)
├── examples/            | 示例程序
│   └── hello_world.vn   | Hello World 示例
├── include/             | 公共头文件
│   ├── ast.h            | 抽象语法树定义
│   └── token.h          | 标记定义
├── LICENSE              | LGPL2.0 许可证文件
├── README.md            | 项目文档 (英文)
├── src/                 | 源代码
│   ├── code_generator.cpp | 代码生成实现
│   ├── code_generator.h   | 代码生成头文件
│   ├── error.cpp          | 错误处理实现
│   ├── error.h            | 错误处理头文件
│   ├── lexer.cpp          | 词法分析器实现
│   ├── lexer.h            | 词法分析器头文件
│   ├── main.cpp           | 主程序入口
│   ├── parser.cpp         | 解析器实现
│   └── parser.h           | 解析器头文件
└── ZH_README.md         | 项目文档 (中文)
```

**重要提示：** 编译模式需要安装 mingw64。确保 `mingw64` 目录与 `vanction.exe` 处于同一目录，或者已在系统 PATH 中正确配置。

## 11. 许可证

本项目采用 LGPL2.0 许可证。

## 12. 贡献

欢迎提交 Issues 和 Pull Requests！

## 13. 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：[VanctionProgramLanguage](https://github.com/bilibili-dideng/VanctionProgramLanguage)
- 邮箱：3483434955@qq.com`
