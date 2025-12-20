# Vanction 编程语言

Vanction是一种现代化的编译型编程语言，采用C++实现，具有简洁的语法和丰富的功能特性。

## 主要特性

- **现代化语法**：简洁、直观的语法设计，易于学习和使用
- **面向对象**：支持类、继承和多态
- **错误处理**：强大的try-happen错误处理机制
- **数据结构**：内置列表（List）和哈希表（HashMap）
- **模块系统**：支持模块导入和命名空间
- **编译型**：通过编译为C++代码，然后使用GCC编译为可执行文件，确保高性能
- **跨平台**：支持Windows等多个平台

## 安装指南

### 系统要求

- Windows 10或更高版本
- CMake 3.10或更高版本
- MinGW-w64 GCC编译器

### 安装步骤

1. **克隆仓库**
   ```bash
   git clone https://github.com/bilibili-dideng/VanctionProgramLanguage.git
   cd VanctionProgramLanguage
   ```

2. **构建项目**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. **运行示例**
   ```bash
   cd ..
   ./build/Vanction examples/test/hello_world.vn
   ```

## 基本使用

### 编译和运行程序

```bash
./build/Vanction <source_file.vn>
```

### 示例程序

```vanction
func main() {
    System.print("Hello, Vanction!");
    return 0;
}
```

## 语言特性

### 变量和常量

```vanction
|| 可变变量
var x = 10;
var name = "Vanction";

|| 不可变常量
immut var PI = 3.14159;
immut var VERSION = "1.0.0";
```

### 函数定义

```vanction
func add(a, b) {
    return a + b;
}

func main() {
    var result = add(5, 3);
    System.print("5 + 3 = " + result);
    return 0;
}
```

### 类和对象

```vanction
class Person {
    var name;
    var age;
    
    func init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    func greet() {
        System.print("Hello, my name is " + this.name + " and I'm " + this.age + " years old.");
    }
}

func main() {
    var person = instance Person("Alice", 30);
    person.greet();
    return 0;
}
```

### 错误处理

```vanction
try {
    var result = 10 / 0;
    System.print("Result: " + result);
} happen (DivideByZeroError) as e {
    System.print("Caught DivideByZeroError: " + e.text);
}
```

### 数据结构

```vanction
|| 列表
var numbers = [1, 2, 3, 4, 5];
numbers.add(6);
System.print("Third element: " + numbers[2]);

|| 哈希表
var person = {
    "name": "Bob",
    "age": 25,
    "city": "New York"
};
System.print("Name: " + person["name"]);
```

## 示例程序

在`examples`目录下提供了多个示例程序，展示了语言的各种特性：

- `hello_world.vn`: 简单的Hello World程序
- `basic_arithmetic.vn`: 基本算术运算
- `conditionals_loops.vn`: 条件语句和循环
- `test_error_handling.vn`: 错误处理机制
- `test_data_structures.vn`: 数据结构使用
- `namespace_clas.vn`: 命名空间和类

## 项目结构

```
VanctionProgramLanguage/
├── src/             # 源代码目录
│   ├── main.cpp     # 主程序入口
│   ├── lexer.cpp    # 词法分析器
│   ├── parser.cpp   # 语法分析器
│   ├── code_generator.cpp  # 代码生成器
│   └── error.cpp    # 错误处理
├── include/         # 头文件目录
├── examples/        # 示例程序
├── build/           # 构建目录
├── mingw64/         # MinGW-w64编译器
├── CMakeLists.txt   # CMake配置文件
└── README.md        # 项目说明文档
```

## 许可证

Vanction编程语言采用GNU Lesser General Public License (LGPL) 2.1，详情请查看LICENSE文件。

## 贡献

欢迎对Vanction编程语言进行贡献！您可以通过以下方式参与：

1. 报告bug
2. 提出新特性建议
3. 提交代码修复或改进
4. 完善文档

## 联系方式

如有问题或建议，请通过GitHub Issues与我们联系。

---

**享受编程的乐趣！** 🚀