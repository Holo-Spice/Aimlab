#ifndef __PID_H__
#define __PID_H__

#include <windows.h>

class PIDController {
public:
    PIDController(double Kp, double Ki, double Kd)
        : Kp(Kp), Ki(Ki), Kd(Kd), lastError(0), integral(0) {}

    // 更新当前位置
    void updatePosition(POINT &current, const POINT &target, double dt);

private:
    double Kp, Ki, Kd;
    double lastError;
    double integral;
};

#endif // __PID_H__