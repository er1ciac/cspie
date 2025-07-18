# 基于 SM3 的 Merkle tree 实现

## 任务描述

基于 SM3 哈希算法的实现，根据RFC6962构建 Merkle 树（支持 10 万叶子节点），并实现叶子节点的存在性证明和不存在性证明。

## 功能特性

- **SM3 哈希算法**: 使用国密 SM3 算法作为 Merkle 树的基础哈希函数。
- **RFC6962 兼容**: Merkle 树的构建遵循 RFC6962 规范。
- **大规模叶子节点**: 支持十万级别（100,000）的叶子节点。
- **存在性证明 (Existence Proof)**: 为 Merkle tree 中的任意叶子节点生成其存在性证明（Merkle Audit Path）。
- **不存在性证明 (Non-existence Proof)**: 为不在 Merkle 树中的数据生成其不存在的证明。

## 编译

使用以下命令来编译本项目：

```bash
g++ main.cpp sm3.cpp merkle.cpp -o sm3

```

## 结果

![](./4c.png)