#include "../include/UART_Parser.h"
#include "../include/protocol_def.h" 
#include "hardware/irq.h" 
#include "pico/time.h"    
#include <stdio.h>
#include <string.h>

// 🌟 声明：这些变量实体在 main.cpp 中
extern volatile float target_pitch_vel;
extern volatile float target_yaw_vel;
extern volatile uint32_t raw_rx_byte_count;
extern volatile uint32_t err_cksm;
extern volatile uint32_t err_tail;

UART_Parser* global_parser_instance = nullptr;

void on_uart_rx_isr() {
    if (global_parser_instance) global_parser_instance->handle_rx_irq();
}

UART_Parser::UART_Parser() : uart_port(nullptr), last_valid_rx_time(0) {}

void UART_Parser::init(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint baud) {
    this->uart_port = uart;
    global_parser_instance = this; 
    uart_init(uart, baud);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart, false, false);
    uart_set_fifo_enabled(uart, false);

    int UART_IRQ = (uart == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx_isr);
    irq_set_priority(UART_IRQ, 0); // 确保通讯最高优先级
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(uart, true, false);

    last_valid_rx_time = to_ms_since_boot(get_absolute_time());
}

void UART_Parser::handle_rx_irq() {
    if (!uart_port) return;
    while (uart_is_readable(uart_port)) {
        uint8_t b = uart_getc(uart_port);
        raw_rx_byte_count++; 
        parseByte(b);
    }
}

uint32_t UART_Parser::getLastRxTime() { return last_valid_rx_time; }

void UART_Parser::parseByte(uint8_t b) {
    switch (currentState) {
        case WAIT_HEAD1:
            if (b == 0x55) currentState = WAIT_HEAD2;
            break;
        case WAIT_HEAD2:
            if (b == 0xAA) currentState = WAIT_CMD;
            else if (b != 0x55) currentState = WAIT_HEAD1;
            break;
        case WAIT_CMD:
            current_cmd = b;
            currentState = WAIT_LEN;
            break;
        case WAIT_LEN:
            target_len = b;
            data_idx = 0;
            if (target_len > 16) currentState = WAIT_HEAD1; 
            else currentState = (target_len > 0) ? STATE_DATA : WAIT_CKSM;
            break;
        case STATE_DATA:
            data_buffer[data_idx++] = b;
            if (data_idx >= target_len) currentState = WAIT_CKSM;
            break;
        case WAIT_CKSM: {
            uint8_t sum = current_cmd + target_len;
            for (int i = 0; i < target_len; i++) sum += data_buffer[i];
            if (sum == b) currentState = WAIT_TAIL;
            else { err_cksm++; currentState = WAIT_HEAD1; }
            break;
        }
        case WAIT_TAIL:
            if (b == 0x0D) {
                PayloadSetVelocity p;
                memcpy(&p, data_buffer, 8);
                target_pitch_vel = p.pitch_vel;
                target_yaw_vel = p.yaw_vel;
                last_valid_rx_time = to_ms_since_boot(get_absolute_time());
            } else { err_tail++; }
            currentState = WAIT_HEAD1;
            break;
    }
}