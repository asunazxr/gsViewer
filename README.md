# 3DGS 实时渲染器（ml_sharp）

基于 **OpenGL** 的 3D Gaussian Splatting 实时渲染器，支持 `.ply` / `.splat` 文件加载、实时渲染、相机交互、离屏渲染和参数调节。

## 功能
- 加载 `.ply` 和 `.splat` 文件  
- 实时渲染 3DGS 模型  
- 鼠标 & 键盘交互控制相机（旋转 / 平移 / 缩放）  
- 使用 **ImGui** 调节渲染参数  
- 异步深度排序（只在相机视角大幅变化时触发）  
- 双缓冲机制保证渲染流畅性  
- 离屏渲染任意分辨率图片  

## 技术栈与依赖库
**OpenGL 渲染与交互**：GLAD, GLFW, GLM, ImGui, KHR  
**3DGS 数据处理**：tinyply-3.0, stb_image.h, stb_image_write  
**构建工具**：CMake  

## 第三方库安装
在构建项目前，请确保已安装以下依赖库：

1. **GLAD** - OpenGL 加载器  
2. **GLFW** - 窗口与输入管理  
3. **GLM** - 数学库（向量/矩阵运算）  
4. **ImGui** - GUI 界面库  
5. **KHR** - OpenGL 扩展支持  
6. **tinyply-3.0** - PLY 文件解析  
7. **stb_image.h / stb_image_write** - 图像读写  

> 可通过系统包管理器或源码编译安装。

## 构建与运行
1. 克隆仓库：
```bash
git clone https://github.com/asunazxr/gsViewer.git
cd ml_sharp
```

2. 创建构建目录并使用 CMake：
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

3. 运行

## 界面展示
<img width="1909" height="998" alt="image" src="https://github.com/user-attachments/assets/ff90c386-e187-4132-a620-458bdf9d36a4" />

### 总结
1. 核心功能：支持多格式加载、实时渲染、相机交互、参数调节、异步深度排序、双缓冲、离屏渲染等核心能力；
2. 技术依赖：基于OpenGL生态（GLAD/GLFW/GLM等）+ 3DGS数据处理库 + CMake构建，需提前安装指定依赖；
3. 构建流程：克隆仓库 → 创建build目录 → CMake配置与编译 → 运行程序，步骤简洁且标准化。
