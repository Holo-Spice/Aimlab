#include "include/tool.h"
#include "include/pid.h"

#include <cmath>
#include <limits>

ID3D11Device *Tool::d3d_device = nullptr;
ID3D11DeviceContext *Tool::d3d_context = nullptr;
IDXGIOutputDuplication *Tool::duplication = nullptr;

void Tool::initScreenCapture() {
    // 创建D3D设备
    D3D_FEATURE_LEVEL featureLevel;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                      D3D11_SDK_VERSION, &d3d_device, &featureLevel, &d3d_context);

    // 获取DXGI接口
    IDXGIDevice *dxgi_device = nullptr;
    d3d_device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgi_device);

    IDXGIAdapter *dxgi_adapter = nullptr;
    dxgi_device->GetAdapter(&dxgi_adapter);

    IDXGIOutput *dxgi_output = nullptr;
    dxgi_adapter->EnumOutputs(0, &dxgi_output);

    IDXGIOutput1 *dxgi_output1 = nullptr;
    dxgi_output->QueryInterface(__uuidof(IDXGIOutput1), (void **)&dxgi_output1);

    // 创建Desktop Duplication
    dxgi_output1->DuplicateOutput(d3d_device, &duplication);

    // 释放临时接口
    dxgi_output1->Release();
    dxgi_output->Release();
    dxgi_adapter->Release();
    dxgi_device->Release();
}

cv::Mat Tool::captureScreen() {
    static cv::Mat screen_mat;
    static cv::Mat bgr_mat;

    IDXGIResource *desktop_resource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frame_info;

    // 尝试获取下一帧
    HRESULT hr = duplication->AcquireNextFrame(0, &frame_info, &desktop_resource);
    if (FAILED(hr)) {
        return bgr_mat; // 返回上一帧的BGR图像，而不是screen_mat
    }

    ID3D11Texture2D *desktop_texture = nullptr;
    hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&desktop_texture);
    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC texture_desc;
        desktop_texture->GetDesc(&texture_desc);

        if (screen_mat.empty() || screen_mat.size() != cv::Size(texture_desc.Width, texture_desc.Height)) {
            screen_mat = cv::Mat(texture_desc.Height, texture_desc.Width, CV_8UC4);
            bgr_mat = cv::Mat(texture_desc.Height, texture_desc.Width, CV_8UC3);
        }

        // 创建可以CPU访问的纹理
        D3D11_TEXTURE2D_DESC cpu_texture_desc = texture_desc;
        cpu_texture_desc.Usage = D3D11_USAGE_STAGING;
        cpu_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        cpu_texture_desc.BindFlags = 0;
        cpu_texture_desc.MiscFlags = 0;

        ID3D11Texture2D *cpu_texture = nullptr;
        hr = d3d_device->CreateTexture2D(&cpu_texture_desc, nullptr, &cpu_texture);
        if (SUCCEEDED(hr)) {
            d3d_context->CopyResource(cpu_texture, desktop_texture);

            D3D11_MAPPED_SUBRESOURCE mapped;
            hr = d3d_context->Map(cpu_texture, 0, D3D11_MAP_READ, 0, &mapped);
            if (SUCCEEDED(hr)) {
                for (UINT i = 0; i < texture_desc.Height; i++) {
                    memcpy(screen_mat.ptr(i),
                           (BYTE *)mapped.pData + i * mapped.RowPitch,
                           texture_desc.Width * 4);
                }
                d3d_context->Unmap(cpu_texture, 0);

                cv::cvtColor(screen_mat, bgr_mat, cv::COLOR_BGRA2BGR);
            }
            cpu_texture->Release();
        }
        desktop_texture->Release();
    }
    desktop_resource->Release();
    duplication->ReleaseFrame();

    return bgr_mat;
}

void Tool::releaseScreenCapture() {
    if (duplication)
        duplication->Release();
    if (d3d_context)
        d3d_context->Release();
    if (d3d_device)
        d3d_device->Release();
}

cv::Mat Tool::preprocessImage(const cv::Mat &rgb_img) {
    static cv::Mat gray_img;
    static cv::Mat hsv;
    static cv::Scalar lower_blue(82, 199, 118);
    static cv::Scalar upper_blue(97, 255, 255);

    cv::cvtColor(rgb_img, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, lower_blue, upper_blue, gray_img);

    return gray_img;
}

void Tool::findtarget(const cv::Mat &binary_img, cv::Mat &rgb_img, std::vector<std::vector<cv::Point>> &contours) {
    using std::vector;
    static std::vector<cv::Vec4i> hierarchy;
    static vector<vector<cv::Point>> filtered_contours;

    contours.clear();
    filtered_contours.clear();
    cv::findContours(binary_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto &contour : contours) {
        if (contour.size() < 4)
            continue;

        cv::Rect r = cv::boundingRect(contour);

        if (r.width * r.height < 1600)
            continue;

        filtered_contours.push_back(contour);

        cv::line(rgb_img, cv::Point(r.x, r.y), cv::Point(r.x + r.width, r.y), cv::Scalar(0, 255, 0), 2);
        cv::line(rgb_img, cv::Point(r.x + r.width, r.y), cv::Point(r.x + r.width, r.y + r.height), cv::Scalar(0, 255, 0), 2);
        cv::line(rgb_img, cv::Point(r.x + r.width, r.y + r.height), cv::Point(r.x, r.y + r.height), cv::Scalar(0, 255, 0), 2);
        cv::line(rgb_img, cv::Point(r.x, r.y + r.height), cv::Point(r.x, r.y), cv::Scalar(0, 255, 0), 2);
    }
    contours = filtered_contours;
}

POINT Tool::getMouse() {
    POINT mouse_point;
    GetCursorPos(&mouse_point);
    return mouse_point;
}

void Tool::moveMouse(int x, int y) {
    static double screen_width = GetSystemMetrics(SM_CXSCREEN) - 1;
    static double screen_height = GetSystemMetrics(SM_CYSCREEN) - 1;

    double dx = x * 65535.0 / screen_width;
    double dy = y * 65535.0 / screen_height;

    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    SendInput(1, &input, sizeof(INPUT));
}

void Tool::moveFromTo(POINT start, POINT end, int dt) {
    PIDController pid(0.091, 0, 0.45); // Kp, Ki, Kd
    POINT current = start;

    while (abs(current.x - end.x) > 12 || abs(current.y - end.y) > 11) {
        pid.updatePosition(current, end, dt);
        moveMouse(current.x, current.y);
    }

    moveMouse(end.x, end.y);
}

void Tool::leftDown() {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}

void Tool::leftUp() {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

cv::Point Tool::findClosestContourCenter(const std::vector<std::vector<cv::Point>> &contours, const POINT &mouse_pos) {
    cv::Point closest_center(-1, -1);
    double min_distance = DBL_MAX;

    for (const auto &contour : contours) {
        if (contour.size() < 4)
            continue;

        // 计算轮廓的中心点
        cv::Rect r = cv::boundingRect(contour);
        cv::Point center(r.x + r.width / 2, r.y + r.height / 2);

        // 计算中心点到鼠标位置的距离
        double distance = std::sqrt(
            std::pow(static_cast<double>(center.x - mouse_pos.x), 2) +
            std::pow(static_cast<double>(center.y - mouse_pos.y), 2));

        // 更新最近的中心点
        if (distance < min_distance) {
            min_distance = distance;
            closest_center = center;
        }
    }

    return closest_center;
}