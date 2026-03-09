#pragma once
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

// 失败就打印错误信息并退出（返回码=1）
inline void __test_fail(const std::string& msg,
                        const char* file,
                        int line) {
    std::cerr << "[FAIL] " << file << ":" << line << " | " << msg << "\n";
    std::exit(1);
}

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            __test_fail(std::string(msg), __FILE__, __LINE__); \
        } \
    } while (0)

#define ASSERT_NEAR(a, b, eps, msg) \
    do { \
        auto __va = (a); \
        auto __vb = (b); \
        auto __diff = std::fabs(__va - __vb); \
        if (__diff > (eps)) { \
            __test_fail(std::string(msg) + \
                        " | got diff=" + std::to_string(__diff) + \
                        " (a=" + std::to_string(__va) + \
                        ", b=" + std::to_string(__vb) + ")", \
                        __FILE__, __LINE__); \
        } \
    } while (0)
