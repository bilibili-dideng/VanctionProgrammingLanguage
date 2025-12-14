# RRP标准-3-System类

## System的用法
System类是Vanction语言的一个系统类，用于系统级别的操作。
System类的方法如下：
```Vanction
System.print(value); || 打印value到标准输出
System.input(value); || value为提示信息，返回值为用户输入的`字符串`
```

如果结合上变量的定义，就可以实现加法的功能。
```Vanction
func main() {
    auto num1 = System.input("请输入第一个数：");
    auto num2 = System.input("请输入第二个数：");
    System.print(f"结果为：{num1 + num2}"); || 格式化字符串
}
```