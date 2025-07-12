# SM3的实现和加速

使用simd的方法来加速SM3

## 使用方法

编译优化版本，在终端中输入命令：

` g++ -mssse3 sm3_simd.cpp main.cpp -o sm3_simd`

编译无优化版本，在终端中输入命令：

` g++ sm3_noop.cpp main.cpp -o sm3_simd`

## 运行结果
以下是SM4算法的执行结果，发现在同时执行100次的时候，查表法比普通版本快：

![SM4 无优化](./noop.png)

![SM4 simd](./res.png)