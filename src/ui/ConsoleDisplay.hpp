#pragma once
#include "IDisplay.hpp"
#include "Mode.hpp"
#include <iostream>

class ConsoleDisplay : public IDisplay {
public:
    void render(const DisplayModel& m) override {
        std::cout << "MODE: " << modeText(m.mode) << "\n";
    }
};