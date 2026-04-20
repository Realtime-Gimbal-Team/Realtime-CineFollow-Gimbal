#include <SimpleFOC.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h" 
#include <math.h>
#include "../include/UART_Parser.h"

// ==============================================================================
// 1. Math Utility Class (静态数学辅助类：封装查表算法，避免全局函数污染)
// ==============================================================================
class FastMath {
private:
    static float sin_lut[256];
    static bool initialized;

public:
    static void init() {
        if (initialized) return;
        for (int i = 0; i < 256; i++) {
            sin_lut[i] = sinf((float)i * 6.2831853f / 256.0f);
        }
        initialized = true;
    }

    static inline float fast_sin(float angle) {
        float angle_norm = fmodf(angle, 6.2831853f);
        if (angle_norm < 0) angle_norm += 6.2831853f;
        int index = (int)(angle_norm * 40.74366f); 
        return sin_lut[index & 0xFF];
    }
};

// 初始化静态成员变量
float FastMath::sin_lut[256];
bool FastMath::initialized = false;

// ==============================================================================
// 2. Gimbal Motor Class (云台电机核心类：彻底消灭 #define 和散装全局变量)
// ==============================================================================
class GimbalMotor {
private:
    // 硬件对象与物理参数 (私有化，禁止外部直接篡改)
    BLDCDriver3PWM driver;
    int en_pin;
    float pole_pairs;
    float voltage_limit;
    
    // 运动状态与平滑器内存
    float smooth_vel;
    float current_angle;
    float current_vol;

public:
    // 构造函数：取代原本的宏定义，实例化时动态注入引脚和参数
    GimbalMotor(int in1, int in2, int in3, int en, float vol_limit, float pp)
        : driver(in1, in2, in3, en), en_pin(en), pole_pairs(pp), voltage_limit(vol_limit),
          smooth_vel(0.0f), current_angle(0.0f), current_vol(0.0f) {}

    // 硬件初始化
    void init(float voltage_supply) {
        gpio_init(en_pin);
        gpio_set_dir(en_pin, GPIO_OUT);
        gpio_put(en_pin, 1);

        driver.voltage_power_supply = voltage_supply;
        driver.init();
        driver.enable();
    }

    // 软启动逻辑：返回 true 代表当前电机的电压已达上限，软启动完成
    bool processSoftStart(float dt) {
        if (current_vol < voltage_limit) {
            current_vol += 2.0f * dt; // Rise rate: 2V/s
            if (current_vol >= voltage_limit) current_vol = voltage_limit;
        }
        return (current_vol >= voltage_limit);
    }

    // 核心 SPWM 运算与输出
    void updateSPWM(float target_vel, float dt, float U_center) {
        // Trapezoidal acceleration limiter
        const float max_accel = 6.0f; 
        const float max_delta_v = max_accel * dt; 

        if (target_vel > smooth_vel + max_delta_v) smooth_vel += max_delta_v; 
        else if (target_vel < smooth_vel - max_delta_v) smooth_vel -= max_delta_v; 
        else smooth_vel = target_vel; 

        // Continuous full-power signal output
        current_angle += smooth_vel * dt;
        float elec_angle = current_angle * pole_pairs;
        
        driver.setPwm(
            U_center + current_vol * FastMath::fast_sin(elec_angle),
            U_center + current_vol * FastMath::fast_sin(elec_angle - 2.0944f), 
            U_center + current_vol * FastMath::fast_sin(elec_angle - 4.1888f)  
        );
    }
};

// ==============================================================================
// 3. System Globals (系统级全局变量：保留给 UART_Parser.cpp 进行跨文件通信)
// ==============================================================================
const float VOLTAGE_SUPPLY = 12.0f;
volatile bool is_soft_starting = true;

// 警告：这些变量保留全局是因为底层的 UART_Parser 可能使用 extern 引用了它们
// 若强行封装会导致外部解析器无法写入数据。在工程中，这称为“合理的通信桥梁妥协”。
volatile float target_pitch_vel = 0.0f;
volatile float target_yaw_vel   = 0.0f; 
volatile uint32_t raw_rx_byte_count = 0;
volatile uint32_t err_cksm = 0;
volatile uint32_t err_tail = 0;

// 实例化面向对象实体 (Instantiate Objects)
GimbalMotor pitchMotor(2, 4, 6, 16, 4.5f, 11.0f);
GimbalMotor yawMotor(10, 11, 12, 15, 4.5f, 11.0f);
UART_Parser uart_parser;

// ==============================================================================
// 4. ISR (硬件定时器中断：逻辑被极限简化，仅负责调用对象的方法)
// ==============================================================================
bool spwm_timer_callback(struct repeating_timer *t) {
    const float dt = 0.002f; 
    const float U_center = VOLTAGE_SUPPLY / 2.0f;

    // 1. 面向对象的软启动检查
    if (is_soft_starting) {
        bool pitch_ready = pitchMotor.processSoftStart(dt);
        bool yaw_ready = yawMotor.processSoftStart(dt);
        if (pitch_ready && yaw_ready) {
            is_soft_starting = false; 
        }
    }

    // 2. 指挥对象执行运算（替代原本的一大坨流水账代码）
    pitchMotor.updateSPWM(target_pitch_vel, dt, U_center);
    yawMotor.updateSPWM(target_yaw_vel, dt, U_center);

    return true; 
}

// ==============================================================================
// 5. Main 
// ==============================================================================
int main() {
    stdio_init_all();
    FastMath::init(); // 初始化静态数学类
    sleep_ms(2000); 

    set_sys_clock_khz(133000, true);

    // 面向对象初始化
    pitchMotor.init(VOLTAGE_SUPPLY);
    yawMotor.init(VOLTAGE_SUPPLY);

    uart_parser.init(uart0, 0, 1, 115200);

    struct repeating_timer timer;
    add_repeating_timer_us(-2000, spwm_timer_callback, NULL, &timer);

    uint32_t last_print_time = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint32_t last_rx = uart_parser.getLastRxTime();

        if (now - last_print_time >= 200) {
            last_print_time = now;
            if ((now - last_rx) > 500) {
                target_pitch_vel = 0.0f;
                target_yaw_vel   = 0.0f;
                printf("[SAFE] Timeout! Bytes: %u | ErrCK: %u\n", raw_rx_byte_count, err_cksm);
            } else {
                printf("[LIVE] P:%.2f Y:%.2f | ErrCK: %u\n", target_pitch_vel, target_yaw_vel, err_cksm);
            }
        }
    }
    return 0;
}