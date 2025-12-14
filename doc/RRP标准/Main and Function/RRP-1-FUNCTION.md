# RRP标准-1-函数

## 主函数
在c语言中，主函数的标准格式如下：
```c
int main() {
    // 程序代码
    return 0; // 可以忽略不写！
}
```
但在Vanction语言中，应当使用主函数的格式如下：
```Vanction
func main() {
    || 程序代码
    return 0; || 可以忽略不写！
}
```

并且retunr的数在解释器中最后返回给操作系统

## 自定义函数
在Vanction语言中，应当使用自定义函数的格式如下：
```Vanction
func function_name(parameter1, parameter2, ...) {
    || 函数体
    return value; || 可以忽略不写！
}
```
然后使用function_name(parameter1, parameter2, ...)来调用函数
