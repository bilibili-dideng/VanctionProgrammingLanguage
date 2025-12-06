# Vanction 编程语言

## 1. 简介

Vanction 是一种简单的编译型编程语言，支持编译为可执行文件（通过内置 GCC）或直接解释执行。

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
char ch = 'a';
string str = "Hello";
bool flag = true;

| 自动类型推断
auto num = 456;
auto str = "World";

| 仅声明不初始化，声明后默认值为undefined
define var;
```

#### 4.2.2 变量赋值

```vanction
num = 789;
str = "New string";
```

### 4.3 数据类型

| 类型 | 描述 | 示例 |
|------|------|------|
| int | 整数 | `int num = 123;` |
| char | 字符 | `char ch = 'a';` |
| string | 字符串 | `string str = "Hello";` |
| bool | 布尔值 | `bool flag = true;` |
| auto | 自动类型推断 | `auto x = 456;` |
| undefined | 未定义值 | `define var;` |

### 4.4 输入输出

#### 4.4.1 输出

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

#### 4.4.2 输入

```vanction
| 获取用户输入，带提示信息
string name = System.input("请输入你的名字：");
System.print("你好，" + name + "!");

| 获取用户输入，不带提示信息
string input = System.input("");
```

## 5. 命令行选项

| 选项 | 描述 |
|------|------|
| `-i <file.vn>` | 解释器模式，直接执行程序 |
| `-g <file.vn>` | 编译模式，生成可执行文件 |
| `-o <output.exe>` | 指定编译输出的可执行文件名 |
| `-h, --help` | 显示帮助信息 |

### 5.1 示例

```powershell
// 解释执行
vanction.exe -i examples/hello_world.vn

// 编译为可执行文件
vanction.exe -g examples/hello_world.vn

// 编译并指定输出文件名
vanction.exe -g examples/hello_world.vn -o hello.exe
```

## 6. 示例程序

### 6.1 变量和输入输出示例

```vanction
func main() {
    | 测试变量声明与初始化功能
    
    | 整数类型
    int num1 = 123;
    System.print(num1);
    
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
    
    | auto类型推断
    auto auto1 = 456;
    System.print(auto1);
    
    auto auto2 = "Auto string";
    System.print(auto2);
    
    | define声明
    define var1;
    
    | 输入功能测试
    string input1 = System.input("请输入你的名字：");
    System.print("你好，" + input1 + "!");
    
    | 输入赋值给变量
    System.print("请输入一个数字：");
    string inputNum = System.input("");
    System.print("你输入的数字是：" + inputNum);
}
```

## 7. 注意事项

1. 所有语句必须以分号 `;` 结尾
2. 字符串字面量使用双引号 `""` 包裹
3. 字符字面量使用单引号 `''` 包裹
4. 布尔值为 `true` 或 `false`（小写）
5. 编译模式需要内置的 GCC 支持，确保 `mingw64` 目录与 `vanction.exe` 处于同一目录
6. 变量名只能包含字母和数字，且不能以数字开头

## 8. 已知限制

1. 目前仅支持基本数据类型
2. 不支持控制流语句（if-else, for, while）
3. 不支持自定义函数
4. 不支持数组和结构体
5. 仅支持单行注释
6. 字符串连接仅支持 `+` 操作符

## 9. 未来计划

- 支持更多数据类型（如浮点数）
- 支持控制流语句
- 支持自定义函数
- 支持数组和结构体
- 支持多行注释
- 添加更多系统函数
- 优化错误提示

## 10. 许可证

本项目采用 MIT 许可证。

## 11. 贡献

欢迎提交 Issues 和 Pull Requests！

## 12. 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：[VanctionProgramLanguage](https://github.com/yourusername/VanctionProgramLanguage)
- 邮箱：your.email@example.com
