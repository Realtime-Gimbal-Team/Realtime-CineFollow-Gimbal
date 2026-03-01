#include <iostream>
#include "Mode.hpp"

int main() {
    Mode m = Mode::Idle;
    std::cout << "MODE: " << modeText(m) << "\n";

    m = Mode::Follow;
    std::cout << "MODE: " << modeText(m) << "\n";

    m = Mode::Lock;
    std::cout << "MODE: " << modeText(m) << "\n";

    m = Mode::Reset;
    std::cout << "MODE: " << modeText(m) << "\n";
    return 0;
}