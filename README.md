# AimLab 自动瞄准工具
由于作者实际上不太会pid，效果可能需要精调，效果且受到分辨率影响
---
效果参考：https://www.bilibili.com/video/BV1WU54zvEha/
## 目录结构

```
aimlab/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── pid.h
│   └── tool.h
├── src/
│   ├── main.cpp
│   ├── pid.cpp
│   └── tool.cpp
└── test/
    ├── test.png
```


### **主程序逻辑**
- **主要逻辑**:
  - 按下 `q` 键开启自瞄。
  - 按下 `e` 键关闭自瞄。
  - 按下 `ESC` 键退出程序。
  - 自动瞄准模式下，鼠标会平滑移动到目标点并点击。

---

## 环境要求

### 1. **操作系统**
- Windows 10 或更高版本（需要支持 DirectX 11）

### 2. **开发环境**
- **编译器**: GCC
- **构建工具**: CMake 3.10 或更高版本

### 3. **依赖库**
- **OpenCV**: 用于图像处理
从 [OpenCV 官网](https://opencv.org/) 下载并配置。
- **DirectX SDK**: 用于屏幕捕获

### 4. **注意事项**
- **屏幕分辨率**:
程序默认捕获整个屏幕。如果需要优化性能，可以修改代码只捕获感兴趣区域（ROI）
- **目标检测**: 
当前目标检测基于 HSV 颜色范围（蓝色）。可以根据需求调整 lower_blue 和 upper_blue 的值。
- **PID 参数**:
由于作者实际上不太会pid，PID 控制器的参数（Kp, Ki, Kd）可能需要根据实际情况调整，以获得最佳的鼠标移动效果。



