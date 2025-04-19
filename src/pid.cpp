#include "include/pid.h"

void PIDController::updatePosition(POINT &current, const POINT &target, double dt) {
    int errorX = target.x - current.x;
    int errorY = target.y - current.y;

    // 比例项
    double pTermX = Kp * errorX;
    double pTermY = Kp * errorY;

    // 积分项
    integral += errorX + errorY;
    double iTermX = Ki * integral * dt;
    double iTermY = Ki * integral * dt;

    // 微分项
    double dTermX = Kd * (errorX - lastError);
    double dTermY = Kd * (errorY - lastError);

    // 更新当前位置
    current.x += pTermX;
    current.y += pTermY;

    // 保存误差
    lastError = errorX + errorY;
}