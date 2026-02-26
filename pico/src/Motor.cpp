#include "../include/Motor.h"
#include <stdio.h>

Motor::Motor(int in1, int in2, int in3, int en) {
    pin_in1 = in1; pin_in2 = in2; pin_in3 = in3; pin_en = en;
    current_angle = 0.0f; target_angle = 0.0f;
}

void Motor::init() {
    printf("[SYSTEM] Motor initialized! Pins: %d,%d,%d, EN:%d\n", pin_in1, pin_in2, pin_in3, pin_en);
}

void Motor::setTargetAngle(float angle) {
    // Safety mechanism: Limit angles to prevent hardware damage
    if (angle > 45.0f) {
        target_angle = 45.0f; 
        printf("[WARN] Angle %.1f too large! Clamped to 45.0 deg\n", angle);
    } else if (angle < -45.0f) {
        target_angle = -45.0f;
        printf("[WARN] Angle %.1f too small! Clamped to -45.0 deg\n", angle);
    } else {
        target_angle = angle;
    }
}

float Motor::getCurrentAngle() { return current_angle; }

void Motor::loopFOC() {
    // Software simulation of FOC operation
    if (current_angle < target_angle) current_angle += 0.5f; 
    else if (current_angle > target_angle) current_angle -= 0.5f;
}