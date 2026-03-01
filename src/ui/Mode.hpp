// Author: yifei
// Target: SSD1306 I2C 128x64
#pragma once
#include <string_view>

enum class Mode{
    Idle,
    Follow,
    Lock,
    Reset
};

inline std::string_view modeText(Mode m) {
    switch (m) {
        case Mode::Follow: return "FOLLOW";
        case Mode::Lock:   return "LOCK";
        case Mode::Idle:   return "IDLE";
        case Mode::Reset:  return "RESET";
        default:           return "UNKNOWN";
    }
}
// Press: FOLLOW to LOCK / LOCK to FOLLOW
// Double Press: RESET
// Long Press: IDLE