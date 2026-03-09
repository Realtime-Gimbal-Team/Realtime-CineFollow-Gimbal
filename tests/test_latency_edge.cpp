#include <iostream>
#include <thread>
#include <chrono>

#include "guardian/LatencyTracker.hpp"
#include "test_util.hpp"   // 你的 ASSERT_TRUE / ASSERT_NEAR 在这里

int main() {
    // 1) Edge: start 后立刻 stop
    {
        LatencyTracker t;
        t.start();
        t.stop();

        auto us = t.us();
        auto ms = t.ms();

        ASSERT_TRUE(us >= 0, "us() should be non-negative");
        ASSERT_TRUE(ms >= 0.0, "ms() should be non-negative");
        ASSERT_NEAR(ms, us / 1000.0, 1e-3, "ms() should match us()/1000");
    }

    // 2) Edge: 连续 stop（有些实现允许，有些不允许；这里我们要求“不崩溃且结果可读”）
    {
        LatencyTracker t;
        t.start();
        t.stop();
        t.stop(); // second stop

        auto us = t.us();
        ASSERT_TRUE(us >= 0, "us() should still be valid after repeated stop()");
    }

    // 3) Small delay: sleep 1ms
    {
        LatencyTracker t;
        t.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        t.stop();

        auto us = t.us();
        ASSERT_TRUE(us > 0, "us() should be > 0 after a delay");
    }

    std::cout << "[OK] test_latency_edge passed\n";
    return 0;
}
