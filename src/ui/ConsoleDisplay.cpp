#include "ConsoleDisplay.hpp"
#include "Mode.hpp"
#include <iostream>

void ConsoleDisplay::render(const DisplayModel& m) {
    std::cout << "MODE: " << modeText(m.mode) << "\n";
}