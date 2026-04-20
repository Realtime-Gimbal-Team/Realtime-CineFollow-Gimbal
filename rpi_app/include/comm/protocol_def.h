#pragma once
#include <stdint.h>

// Disable compiler structure alignment optimizations to prevent data misalignment caused by automatic padding across different compilers [89, 94, 95].
#pragma pack(push, 1)

// Basic frame header definition [96].
struct FrameHeader {
    uint8_t head1 = 0x55;      // Frame Header 1 [44, 98]
    uint8_t head2 = 0xAA;      // Frame Header 2 [44, 99]
    uint8_t cmd_id;            // Command function code [44, 100]
    uint8_t data_len;          // Payload length [44, 101]
};

// CMD 0x01: Set target angle payload [48, 104]
struct PayloadSetAngle {
    float pitch;               // Pitch angle (unit: degrees) [53, 106]
    float yaw;                 // Yaw angle (°) [54, 107]
};

//Complete data frame for sending target angles [123, 124]
struct FrameSetAngle {
    FrameHeader header;        // [125]
    PayloadSetAngle payload;   // [126]
    uint8_t checksum;          // Checksum [44, 127]
    uint8_t tail = 0x0D;       // Frame tail: fixed as 0x0D (\r) [44, 128]
};

// Restore the compiler's default alignment [130].
#pragma pack(pop)
