#include <iostream>
#include <cassert>
#include "guardian/LatencyTracker.hpp"

int main() {
    LatencyTracker lt;

    // 1. 基本调用测试
    lt.start();
    lt.stop();

    // 2. 高频循环稳定性测试（模拟控制循环）
    for (int i = 0; i < 100000; i++) {
        lt.start();
        lt.stop();
    }

    std::cout << "[PASS] LatencyTracker stability test\n";
    return 0;
}
