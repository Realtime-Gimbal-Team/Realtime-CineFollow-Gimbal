#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Ssd1306Display {
public:

    Ssd1306Display(const char* i2c_dev = "/dev/i2c-1", uint8_t addr = 0x3C);
    ~Ssd1306Display();

    bool init();   
    void clear();  
    void drawText(int x, int y, const std::string& text); 
    bool flush();  
private:
    bool writeCommand(std::initializer_list<uint8_t> cmds);
    bool writeData(const uint8_t* data, size_t len);

    void setPixel(int x, int y, bool on);

private:
    int fd_{-1};
    uint8_t addr_{0x3C};
    std::vector<uint8_t> fb_; // 128*64/8 = 1024 bytes
};