#include <SimpleFOC.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h" 
#include <math.h>
#include "../include/UART_Parser.h"

//Pin and hardware configuration
#define PITCH_IN1 2
#define PITCH_IN2 4
#define PITCH_IN3 6
#define PITCH_EN  16 

#define YAW_IN1   10
#define YAW_IN2   11
#define YAW_IN3   12
#define YAW_EN    15 

// Physical motion parameters
const float POLE_PAIRS = 11.0f;
const float VOLTAGE_SUPPLY = 12.0f;
const float PITCH_VOL_LIMIT = 4.5f; 
const float YAW_VOL_LIMIT   = 4.5f; 

volatile float target_pitch_vel = 0.0f;
volatile float target_yaw_vel   = 0.0f; 
volatile float current_pitch_angle = 0.0f; 
volatile float current_yaw_angle = 0.0f;  

// Diagnostic variables
volatile uint32_t raw_rx_byte_count = 0;
volatile uint32_t err_cksm = 0;
volatile uint32_t err_tail = 0;

// Physical smoothing and soft-start variables
volatile bool is_soft_starting = true;
volatile float current_pitch_vol = 0.0f; 
volatile float current_yaw_vol = 0.0f;
volatile float smooth_pitch_vel = 0.0f;  
volatile float smooth_yaw_vel = 0.0f;

// Ultra-fast sine lookup table
static float sin_lut[256];
void init_sin_lut() {
    for (int i = 0; i < 256; i++) {
        sin_lut[i] = sinf((float)i * 6.2831853f / 256.0f);
    }
}

inline float fast_sin(float angle) {
    float angle_norm = fmodf(angle, 6.2831853f);
    if (angle_norm < 0) angle_norm += 6.2831853f;
    int index = (int)(angle_norm * 40.74366f); 
    return sin_lut[index & 0xFF];
}

BLDCDriver3PWM pitch_driver(PITCH_IN1, PITCH_IN2, PITCH_IN3, PITCH_EN);
BLDCDriver3PWM yaw_driver(YAW_IN1, YAW_IN2, YAW_IN3, YAW_EN);
UART_Parser uart_parser;

// 500 Hz hard real-time control interrupt
bool spwm_timer_callback(struct repeating_timer *t) {
    const float dt = 0.002f; 
    const float U_center = VOLTAGE_SUPPLY / 2.0f;

    // 1. True soft alignment
    if (is_soft_starting) {
        current_pitch_vol += 2.0f * dt; // Rise rate：2V/s
        current_yaw_vol += 2.0f * dt;
        if (current_pitch_vol >= PITCH_VOL_LIMIT) current_pitch_vol = PITCH_VOL_LIMIT;
        if (current_yaw_vol >= YAW_VOL_LIMIT) current_yaw_vol = YAW_VOL_LIMIT;
        if (current_pitch_vol == PITCH_VOL_LIMIT && current_yaw_vol == YAW_VOL_LIMIT) {
            is_soft_starting = false; 
        }
    }

    //  2. Trapezoidal acceleration limiter
    const float max_accel = 6.0f; 
    const float max_delta_v = max_accel * dt; 

    if (target_pitch_vel > smooth_pitch_vel + max_delta_v) smooth_pitch_vel += max_delta_v; 
    else if (target_pitch_vel < smooth_pitch_vel - max_delta_v) smooth_pitch_vel -= max_delta_v; 
    else smooth_pitch_vel = target_pitch_vel; 

    if (target_yaw_vel > smooth_yaw_vel + max_delta_v) smooth_yaw_vel += max_delta_v;
    else if (target_yaw_vel < smooth_yaw_vel - max_delta_v) smooth_yaw_vel -= max_delta_v;
    else smooth_yaw_vel = target_yaw_vel;

    //  3. Continuous full-power signal output
    current_pitch_angle += smooth_pitch_vel * dt;
    float p_elec = current_pitch_angle * POLE_PAIRS;
    float p_amp = current_pitch_vol; 
    
    pitch_driver.setPwm(
        U_center + p_amp * fast_sin(p_elec),
        U_center + p_amp * fast_sin(p_elec - 2.0944f), 
        U_center + p_amp * fast_sin(p_elec - 4.1888f)  
    );

    current_yaw_angle += smooth_yaw_vel * dt;
    float y_elec = current_yaw_angle * POLE_PAIRS;
    float y_amp = current_yaw_vol; 

    yaw_driver.setPwm(
        U_center + y_amp * fast_sin(y_elec),
        U_center + y_amp * fast_sin(y_elec - 2.0944f),
        U_center + y_amp * fast_sin(y_elec - 4.1888f)
    );

    return true; 
}

int main() {
    stdio_init_all();
    init_sin_lut(); 
    sleep_ms(2000); 

    set_sys_clock_khz(133000, true);

    gpio_init(PITCH_EN); gpio_set_dir(PITCH_EN, GPIO_OUT); gpio_put(PITCH_EN, 1);
    gpio_init(YAW_EN);   gpio_set_dir(YAW_EN,   GPIO_OUT); gpio_put(YAW_EN,   1);

    pitch_driver.voltage_power_supply = VOLTAGE_SUPPLY;
    pitch_driver.init(); 
    pitch_driver.enable();

    yaw_driver.voltage_power_supply = VOLTAGE_SUPPLY;
    yaw_driver.init(); 
    yaw_driver.enable();

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