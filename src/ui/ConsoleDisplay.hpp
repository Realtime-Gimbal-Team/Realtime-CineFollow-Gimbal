#pragma once
#include "IDisplay.hpp"

class ConsoleDisplay : public IDisplay {
public:
    void render(const DisplayModel& m) override;
};