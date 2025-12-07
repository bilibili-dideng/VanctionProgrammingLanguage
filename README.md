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
    return 0;
}
```

`main` 函数中的 `return` 语句在解释器模式下将作为程序的退出码。

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

`main` 函数可以返回一个整数值，在解释器模式下运行时，该值将作为程序的退出码。如果没有提供 return 语句，程序将默认以 0 退出。

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

|| 示例
string s = "abc";
for (char c in s) {
    System.print(c);
}

|| 列表迭代示例
List list = [1, 2, 3];
for (int i in list) {
    System.print(i);
}

|| 哈希表迭代示例
HashMap map = {"a"="1", "b"="2"};
for (key, value in map) {
    System.print("Key: " + key + ", Value: " + value);
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

#### 4.5.6 switch 语句

```vanction
|| switch 语句
switch(表达式) {
    case 值1 {
        || 代码块1
    }
    case 值2 {
        || 代码块2
    }
    // 更多 case...
}

|| 示例
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

#### 4.5.7 return 语句

`return` 语句用于退出函数并可选地返回一个值。在 `main` 函数中，返回值在解释器模式下用作程序的退出码。

```vanction
|| 不带表达式的 return 语句
return;

|| 带表达式的 return 语句
return 表达式;

|| main 函数中的示例
func main() {
    System.print("Hello, Vanction!");
    return 0;  // 以 0 退出
}
```

### 4.6 类型转换

Vanction 提供了类型转换函数，用于在不同数据类型之间转换：

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

### 4.8 命名空间

命名空间用于组织代码并避免命名冲突，允许你将相关函数分组到一个共同的名称下。

#### 4.8.1 命名空间声明

```vanction
|| 命名空间声明语法
namespace 命名空间名称 {
    || 函数和其他声明放在这里
}

|| 示例
namespace Math {
    func add(a, b) {
        return a + b;
    }
    
    func multiply(a, b) {
        return a * b;
    }
}
```

#### 4.8.2 访问命名空间成员

要访问命名空间中的函数或变量，使用冒号运算符 `:`：

```vanction
|| 访问命名空间成员
result = 命名空间名称:成员名称;

|| 示例
func main() {
    int sum = Math:add(3, 4);
    int product = Math:multiply(3, 4);
    System.print("Sum: ", sum);
    System.print("Product: ", product);
}
```

### 4.9 面向对象编程 (OOP)

Vanction 支持基本的面向对象编程特性，包括类定义、继承、实例方法、类方法和实例变量。

#### 4.9.1 类定义

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

|| 示例：基类 Person
class Person() {
    instance.init(instance, name, age) {
        instance.name = name;
        instance.age = age;
    }
    
    instance.getName(instance) {
        return instance.name;
    }
    
    instance.getAge(instance) {
        return instance.age;
    }
    
    class.greet() {
        System.print("Hello from Person class!");
    }
}
```

#### 4.9.2 类继承

```vanction
|| 派生类声明语法
class 派生类名(父类名) {
    || 重写 init 方法
    instance.init(instance, 参数) {
        || 调用父类 init 方法
        父类名.init(instance, 父类参数);
        || 初始化额外的实例变量
        instance.派生变量 = 值;
    }
    
    || 重写实例方法
    instance.方法名(instance, 参数) {
        || 方法体
        return 结果;
    }
}

|| 示例：派生类 Student 继承自 Person
class Student(Person) {
    instance.init(instance, name, age, studentId) {
        || 调用父类构造函数
        Person.init(instance, name, age);
        instance.studentId = studentId;
    }
    
    instance.getStudentId(instance) {
        return instance.studentId;
    }
    
    || 重写父类方法
    instance.getName(instance) {
        return "Student: " + instance.name;
    }
}
```

#### 4.9.3 实例创建

```vanction
|| 创建实例语法
变量 = instance 类名(参数);

|| 示例
func main() {
    || 创建 Person 实例
    person1 = instance Person("Alice", 30);
    
    || 创建 Student 实例
    student1 = instance Student("Bob", 20, "S12345");
}
```

#### 4.9.4 实例方法调用

```vanction
|| 实例方法调用语法
结果 = 实例名.方法名(参数);

|| 示例
func main() {
    person1 = instance Person("Alice", 30);
    
    || 调用实例方法
    string name = person1.getName();
    int age = person1.getAge();
    
    System.print("Name: " + name);
    System.print("Age: " + age);
}
```

#### 4.9.5 类方法调用

```vanction
|| 类方法调用语法
class.方法名();

|| 示例
func main() {
    || 调用类方法
    class.greet();
}
```

#### 4.9.6 实例变量访问

```vanction
|| 访问实例变量
值 = 实例名.变量名;

|| 修改实例变量
实例名.变量名 = 新值;

|| 示例
func main() {
    person1 = instance Person("Alice", 30);
    
    || 访问实例变量
    System.print("Current age: " + person1.age);
    
    || 修改实例变量
    person1.age = 31;
    System.print("Updated age: " + person1.age);
}
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
| `MethodError` | 方法调用错误（例如，调用不存在的方法） |
| `ValueError` | 类型转换错误（例如将字符串 "abc" 转换为整数） |
| `TokenError` | 未知标记错误 |
| `SyntaxError` | 语法错误 |
| `MainFunctionError` | 缺少主函数 |
| `UnknownError` | 未知错误 |

### 5.2 错误信息示例

```
error occurred to:
    C:\Users\xxx\test_error.vn:15:3

    func main() {
        int a = 10 / 0;
    ^^^^^^^
DivideByZeroError: Division by zero
}
```

### 5.3 try-happen 错误处理

Vanction 支持 try-happen 错误处理结构，允许你在运行时捕获和处理错误。语法如下：

```vanction
try {
    || 可能抛出错误的代码
} happen (ErrorType) as errorVariable {
    || 处理错误的代码
    System.print(errorVariable.text);
    System.print(errorVariable.type);
    System.print(errorVariable.info);
}
```

#### 5.3.1 错误对象属性

`happen` 块中的错误对象具有以下属性：

- `text`：完整错误信息，格式为 "XXXError: 为什么报错"
- `type`：错误类型标识符（即 "XXXError" 部分）
- `info`：错误详情描述（即 "为什么报错" 部分）

#### 5.3.2 try-happen 示例

```vanction
func main() {
    System.print("Testing try-happen error handling...");
    
    || 测试除零错误
    try {
        System.print("Test 1: Division by zero");
        auto result = 10 / 0;
        System.print("Result: " + result);
    } happen (DivideByZeroError) as e {
        System.print("Caught DivideByZeroError:");
        System.print("  Error text: " + e.text);
        System.print("  Error type: " + e.type);
        System.print("  Error info: " + e.info);
    }
    
    || 测试值错误
    try {
        System.print("\nTest 2: Value error");
        auto result = type.int("abc");
        System.print("Result: " + result);
    } happen (ValueError) as e {
        System.print("Caught ValueError:");
        System.print("  Error text: " + e.text);
        System.print("  Error type: " + e.type);
        System.print("  Error info: " + e.info);
    }
    
    || 测试通用错误捕获
    try {
        System.print("\nTest 3: Generic Error type");
        auto result = 10 / 0;
        System.print("Result: " + result);
    } happen (ErrorType) as e {
        System.print("Caught generic Error:");
        System.print("  Error text: " + e.text);
        System.print("  Error type: " + e.type);
        System.print("  Error info: " + e.info);
    }
    
    System.print("\nAll tests completed!");
    return 0;
}
```

## 6. 命令行选项

| 选项 | 描述 |
|------|------|
| `-i <file.vn>` | 解释器模式，直接执行程序 |
| `-g <file.vn>` | 编译模式，生成可执行文件 |
| `-o <output.exe>` | 指定编译输出的可执行文件名 |
| `-debug` | 启用词法分析器、解析器、主程序和代码生成器的调试日志 |
| `-config <key> [set <value>\|\|get\|\|reset]` | 配置程序设置（例如：-config GCC set <path>） |
| `-h, --help` | 显示帮助信息 |

### 6.1 示例

```powershell
// 解释执行
vanction.exe -i examples/hello_world.vn

// 编译为可执行文件
vanction.exe -g examples/hello_world.vn

// 编译并指定输出文件名
vanction.exe -g examples/hello_world.vn -o hello.exe

// 获取 GCC 路径配置
vanction.exe -config GCC get

// 设置 GCC 路径配置
vanction.exe -config GCC set "path/to/g++.exe"

// 重置 GCC 路径为默认值
vanction.exe -config GCC reset
```

## 7. 示例程序

### 7.1 变量和输入输出示例

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
    
    || 位运算
    System.print(8 << 1);
    System.print(8 >> 1);
    
    || 逻辑运算
    System.print(flag1 & flag2);
    System.print(flag1 | flag2);
    System.print(flag1 ^ flag2);
    
    || 类型转换
    int i = 42;
    float f2 = type.float(i);
    System.print(f2);
    
    || 输入功能测试
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

1. 仅支持单行注释
2. 字符串操作仅支持 `+` 拼接运算符
3. List类型仅支持整数元素
4. HashMap类型仅支持字符串键值对

## 10. 未来计划

- 支持多行注释
- 添加更多系统函数

## 11. 项目结构

```
VanctionProgramLanguage/
├── bin/                 || 二进制输出目录
├── CMakeLists.txt       || CMake 配置文件
├── doc/                 || 文档目录
│   ├── DEVELOPMENT.md   || 开发文档 (英文)
│   └── ZH_DEVELOPMENT.md|| 开发文档 (中文)
├── examples/            || 示例程序
│   ├── hello_world.vn   || Hello World 示例
│   └── namespace_test.vn || 命名空间功能示例
├── include/             || 公共头文件
│   ├── ast.h            || 抽象语法树定义
│   └── token.h          || 标记定义
├── LICENSE              || LGPL2.0 许可证文件
├── README.md            || 项目文档 (英文)
├── src/                 || 源代码
│   ├── code_generator.cpp || 代码生成实现
│   ├── code_generator.h   || 代码生成头文件
│   ├── error.cpp          || 错误处理实现
│   ├── error.h            || 错误处理头文件
│   ├── lexer.cpp          || 词法分析器实现
│   ├── lexer.h            || 词法分析器头文件
│   ├── main.cpp           || 主程序入口
│   ├── parser.cpp         || 解析器实现
│   └── parser.h           || 解析器头文件
└── ZH_README.md         || 项目文档 (中文)
```

**重要提示：** 编译模式需要安装 mingw64。确保 `mingw64` 目录与 `vanction.exe` 处于同一目录

## 12. 许可证

本项目采用 LGPL2.0 许可证。

## 13. 贡献

欢迎提交 Issues 和 Pull Requests！

## 14. 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：[VanctionProgramLanguage](https://github.com/bilibili-dideng/VanctionProgrammingLanguage)
- 邮箱：3483434955@qq.com
