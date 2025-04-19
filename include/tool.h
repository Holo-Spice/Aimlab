#ifndef __TOOL_H__
#define __TOOL_H__

#include <chrono>
#include <opencv2/opencv.hpp>
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class Tool {

public:
    // 获取当前鼠标坐标
    static POINT getMouse();

    // 移动鼠标到指定位置
    static void moveMouse(int x, int y);

    // 从起点平滑移动鼠标到目标点
    static void moveFromTo(POINT start, POINT end, int dt);

    // 左键按下
    static void leftDown();

    // 左键释放
    static void leftUp();

    // 初始化屏幕捕获（DirectX）
    static void initScreenCapture();

    // 释放屏幕捕获资源
    static void releaseScreenCapture();

    // 捕获屏幕图像并返回为OpenCV的Mat格式
    static cv::Mat captureScreen();

    // 预处理图像
    static cv::Mat preprocessImage(const cv::Mat &rgb_img);

    // 查找目标
    static void findtarget(const cv::Mat &binary_img, cv::Mat &rgb_img, std::vector<std::vector<cv::Point>> &contours);

    // 计算距离鼠标最近的轮廓中心点
    static cv::Point findClosestContourCenter(const std::vector<std::vector<cv::Point>> &contours, const POINT &mouse_pos);

private:
    Tool() = default;
    // Direct3D设备
    static ID3D11Device *d3d_device;
    // Direct3D设备上下文
    static ID3D11DeviceContext *d3d_context;
    // 屏幕复制接口
    static IDXGIOutputDuplication *duplication;
};

#endif // __TOOL_H__
