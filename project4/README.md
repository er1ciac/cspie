# SM3的实现和加速

本项目实现了国密SM3散列算法的标准版本和SIMD优化版本，通过使用SIMD指令集来加速SM3算法的执行。

## 项目概述

SM3是中华人民共和国国家密码管理局制定的密码散列函数标准，产生256位散列值。本项目包含两个版本的实现：

- **标准版本** (`sm3_noop.cpp`)：基础的SM3算法实现，未使用任何优化
- **SIMD优化版本** (`sm3_simd.cpp`)：使用SIMD优化的SM3算法实现

## 文件结构

```
project4/
├── main.cpp          # 主程序文件，包含性能测试代码
├── sm3.h             # SM3算法头文件，定义接口和Timer类
├── sm3_noop.cpp      # SM3标准实现（无优化）
├── sm3_simd.cpp      # SM3 SIMD优化实现
```

## 使用方法

### 编译

编译SIMD优化版本：
```bash
g++ -mssse3 sm3_simd.cpp main.cpp -o sm3_simd
```

编译标准版本（无优化）：
```bash
g++ sm3_noop.cpp main.cpp -o sm3_noop
```
## 运行结果

通过对比测试发现，在执行10000次散列计算时，SIMD优化版本相比标准版本有显著的性能提升。

**标准版本（无优化）：**

![SM3 无优化](./noop.png)

**SIMD优化版本：**

![SM3 SIMD](./res.png)