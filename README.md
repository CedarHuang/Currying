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
auto c2 = c(2)(3.3);
```
这时候, `c2` 依旧是一个柯里化对象, 还没有进行实际调用.

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
而第二种方式的存在, 是因为 参数列表为空的柯里化对象 具有一个到返回值的隐式转换.
设计第二种方式的原因很简单:
```cpp
std::cout << c(2)(3.3) << std::endl;
```
我不希望如上代码会因为没有第二种方式的原因写成这样:
```cpp
std::cout << c(2)(3.3)() << std::endl;
```
需要注意的是, 在没有对 `c2` 进行调用操作, 也就是说没有调用 `operator()` 或 `operator R` 时, **不会做 对 *被柯里化的可调用对象* 的调用操作**!  
这一切都是惰性的!

## 注意事项
```cpp
double f(int &i, double d) {
    return i + d;
}

_currying_impl<double(int &, double), std::tuple<int &>, double>
create() {
    int i = 2;
    return make_currying(f)(i);
}
```
使用如上 `create()` 构建的柯里化对象将导致未定义行为, 因为存在 **悬垂引用**!
若 `f` 的函数签名改为
```cpp
double f(const int &i, double d);
```
依旧会形成 **悬垂引用**, 导致未定义行为.

## 后续优化
1. 我希望我在将来能有能力优化形参为 `const &` 的情况下的柯里化行为, 使用户可定义柯里化类内部是否存储一份副本, 以避免悬垂引用的产生.
2. 我注意到柯里化类的类型非常复杂, 导致在没有 `C++14` 支持的情况下 (`C++14` 的返回值可写 `auto`) 想返回一个柯里化对象时有巨大的不便, 我将简化它.

## 谢谢.