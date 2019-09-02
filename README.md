# Currying
基于 C++11 的可调用对象柯里化库

## 如何使用

### 包含头文件
```cpp
#include "currying.hpp"
```

### 将可调用对象柯里化
```cpp
double func(int i, double d) {
    return i + d;
}
```
对 `func` 柯里化:
```cpp
cedar::currying<double(int, double)> c(func);
```
或
```cpp
auto c = cedar::make_currying(func);
```
两种方式.

### 传参
```cpp
auto c1 = c(2);
auto c2 = c1(3.3);
```
亦可
```cpp
auto c2 = (2)(3.3);
```
这时候, `c2` 依旧是一个柯里化类, 还没有进行实际调用.

### 调用
```cpp
std::cout << c2() << std::endl;
```
或
```cpp
std::cout << c2 << std::endl;
```
得
```
5.3
```
第一种方式的理解, 是因为 `c2` 依旧是个可调用对象, 只不过参数列表为空.  
而第二种方式的存在, 是因为 参数列表为空的柯里化类 具有一个到返回值的隐式转换.
设计第二种方式的原因很简单:
```cpp
std::cout << c(2)(3.3) << std::endl;
```
我不希望如上代码会因为没有第二种方式的原因写成这样:
```cpp
std::cout << c(2)(3.3)() << std::endl;
```
需要注意的是, 在没有对 `c2` 进行调用操作, 也就是说没有调用 `operator()` 或 `operator R` 时, **不会做 对被柯里化的可调用对象的调用操作**!  
这一切都是惰性的!

## 谢谢.