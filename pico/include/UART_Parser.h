#pragma once
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "protocol_def.h"
#include "Motor.h"

class UART_Parser {
public:
    UART_Parser(Motor* p, Motor* y);
    void init(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint baud);
    void spinOnce(); // 在主循环中高频调用

private:
    void parseByte(uint8_t byte);
    
    uart_inst_t* uart_port;
    Motor* pitch_motor;
    Motor* yaw_motor;

    enum State { WAIT_HEAD1, WAIT_HEAD2, WAIT_CMD, WAIT_LEN, STATE_DATA, WAIT_CKSM, WAIT_TAIL };
    State currentState = WAIT_HEAD1;
    
    uint8_t data_buffer[32];
    int data_idx = 0;
    uint8_t target_len = 0;
    uint8_t current_cmd = 0;
};