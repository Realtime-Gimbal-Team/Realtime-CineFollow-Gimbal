#pragma once
#include "pico/stdlib.h"
#include "hardware/uart.h"

// 彻底删除 class Motor 的前置声明

enum ParserState { WAIT_HEAD1, WAIT_HEAD2, WAIT_CMD, WAIT_LEN, STATE_DATA, WAIT_CKSM, WAIT_TAIL };

class UART_Parser {
private:
    // 彻底删除 pitch_motor 和 yaw_motor 的指针成员
    uart_inst_t* uart_port;

    ParserState currentState = WAIT_HEAD1;
    uint8_t current_cmd = 0;
    uint8_t target_len = 0;
    uint8_t data_idx = 0;
    uint8_t data_buffer[16];

    // 通讯安全看门狗的时间戳
    volatile uint32_t last_valid_rx_time; 

    void parseByte(uint8_t b);

public:
    // 构造函数不再需要传入任何参数
    UART_Parser(); 
    void init(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint baud);
    void handle_rx_irq();

    uint32_t getLastRxTime();
};