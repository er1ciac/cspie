# 水印嵌入和提取
首先使用DWT实现图片水印嵌入，其次
根据水印嵌入配套的DWT水印提取，最后
进行翻转、平移等鲁棒性测试。

![水印](./res.png)
## 如何运行
opencv 库:

`sudo apt install libopencv-dev `

`sudo apt update`

编译代码:

```bash
g++ -std=c++11 -o  extract  extract.cpp `pkg-config --cflags --libs opencv4`

 g++ -std=c++11 -o insert insert.cpp `pkg-config --cflags --libs opencv4`
```
