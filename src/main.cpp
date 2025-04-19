#include "include/tool.h"

int main() {
    Tool::initScreenCapture();

    cv::namedWindow("Screen Capture", cv::WINDOW_KEEPRATIO);
    cv::namedWindow("gray_screen", cv::WINDOW_KEEPRATIO);
    std::vector<std::vector<cv::Point>> contours;
    bool autoAim = false;
    auto autoAim_start = std::chrono::steady_clock::now();
    const int AUTO_AIM_DURATION = 70000; // 自瞄运行70s 防止卡死无法退出

    while (true) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        // cv::Mat screen = cv::imread("../test/test.png");
        cv::Mat screen = Tool::captureScreen();

        cv::Mat binary_img = Tool::preprocessImage(screen);
        Tool::findtarget(binary_img, screen, contours);

        POINT mouse_pos = Tool::getMouse();
        cv::Point closest_center = Tool::findClosestContourCenter(contours, mouse_pos);

        int key = cv::waitKey(1);
        if (key == 'q') {
            autoAim = true;
            // 记录开启自瞄的时间
            autoAim_start = std::chrono::steady_clock::now();
            Sleep(2000);
        } else if (key == 'e') {
            autoAim = false;
        } else if (key == 27) {
            break;
        }

        if (autoAim) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                               current_time - autoAim_start)
                               .count();

            if (elapsed >= AUTO_AIM_DURATION) {
                autoAim = false;
                cv::putText(screen, "Auto-aim timeout!", cv::Point(10, 150),
                            cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
            }
        }

        if (autoAim && closest_center.x != -1 && closest_center.y != -1) {
            POINT target = {closest_center.x, closest_center.y};
            POINT current = Tool::getMouse();

            // 计算距离
            double distance = std::sqrt(
                std::pow(current.x - target.x, 2) +
                std::pow(current.y - target.y, 2));

            // 如果距离足够小，执行点击
            if (distance < 36) {
                Tool::leftDown();
                Sleep(1);
                Tool::leftUp();
            }
            // 否则移动鼠标
            else {
                Tool::moveFromTo(current, target, 0.001);
            }
        }

        cv::circle(screen, closest_center, 10, cv::Scalar(255, 0, 0), -1);

        auto frame_end = std::chrono::high_resolution_clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);

        std::string frame_text = "Lantency: " + std::to_string(frame_duration.count()) + " ms";
        cv::putText(screen, frame_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                    1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow("gray_screen", binary_img);
        cv::imshow("Screen Capture", screen);
    }

    cv::destroyAllWindows();
    Tool::releaseScreenCapture();
    return 0;
}