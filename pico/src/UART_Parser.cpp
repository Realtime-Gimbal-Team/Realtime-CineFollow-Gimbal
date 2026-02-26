#include "../include/UART_Parser.h"
#include <stdio.h>
#include <string.h>

UART_Parser::UART_Parser(Motor* p, Motor* y) : pitch_motor(p), yaw_motor(y), uart_port(nullptr) {}

void UART_Parser::init(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint baud) {
    this->uart_port = uart; // 这里现在接收的是 uart1
    uart_init(uart, baud);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart, false, false);
}

void UART_Parser::spinOnce() {
    if (!uart_port) return;
    while (uart_is_readable(uart_port)) {
        uint8_t b = uart_getc(uart_port);
        // 如果想看树莓派发来的原始 16 进制，可以开启下面这行
        // printf("%02X ", b); fflush(stdout);
        parseByte(b);
    }
}

void UART_Parser::parseByte(uint8_t b) {
    switch (currentState) {
        case WAIT_HEAD1:
            if (b == 0x55) currentState = WAIT_HEAD2;
            break;
        case WAIT_HEAD2:
            if (b == 0xAA) currentState = WAIT_CMD;
            else currentState = WAIT_HEAD1;
            break;
        case WAIT_CMD:
            current_cmd = b;
            currentState = WAIT_LEN;
            break;
        case WAIT_LEN:
            target_len = b;
            data_idx = 0;
            currentState = (target_len > 0) ? STATE_DATA : WAIT_CKSM;
            break;
        case STATE_DATA:
            data_buffer[data_idx++] = b;
            if (data_idx >= target_len) currentState = WAIT_CKSM;
            break;
        case WAIT_CKSM: {
            uint8_t sum = current_cmd + target_len;
            for (int i = 0; i < target_len; i++) sum += data_buffer[i];
            if (sum == b) currentState = WAIT_TAIL;
            else currentState = WAIT_HEAD1;
            break;
        }
        case WAIT_TAIL:
            if (b == 0x0D) {
                PayloadSetAngle p;
                memcpy(&p, data_buffer, 8);
                pitch_motor->setTargetAngle(p.pitch);
                yaw_motor->setTargetAngle(p.yaw);
                printf("\n[V1.2 RX SUCCESS] P:%.2f, Y:%.2f", p.pitch, p.yaw);
            }
            currentState = WAIT_HEAD1;
            break;
    }
}