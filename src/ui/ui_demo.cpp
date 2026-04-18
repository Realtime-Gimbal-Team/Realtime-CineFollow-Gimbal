#include <iostream>
#include <string>
#include <chrono>

#include "ButtonLogic.hpp"
#include "ModeManager.hpp"
#include "ConsoleDisplay.hpp"
#include "DisplayModel.hpp"
#include "ButtonEvent.hpp"

int main() {
    using Clock = ButtonLogic::Clock;

    ConsoleDisplay display;
    DisplayModel model;

    ModeManager mm;
    mm.subscribe([&](Mode, Mode newM){
        model.mode = newM;
        display.render(model);
    });

    ButtonConfig cfg;
    cfg.debounce = std::chrono::milliseconds(30);
    cfg.double_click_window = std::chrono::milliseconds(500);
    cfg.long_press_threshold = std::chrono::milliseconds(1000);

    ButtonLogic button(cfg, [&](ButtonEvent e){
        mm.onButtonEvent(static_cast<int>(e));
    });

    std::cout << "One-button demo (simulate GPIO):\n"
              << "  p = press\n"
              << "  r = release\n"
              << "  q = quit\n"
              << "Tips:\n"
              << "  click:  p then r\n"
              << "  double: p r p r (within 0.5s)\n"
              << "  long:   p (hold >1s) then r (or wait, poll will fire)\n\n";

    display.render(model);

    for (std::string s; std::getline(std::cin, s); ) {
        auto now = Clock::now();
        button.poll(now);

        if (s.empty()) continue;
        char c = s[0];
        if (c == 'q') break;

        if (c == 'p') {
            button.onLevelChanged(true, Clock::now());
        } else if (c == 'r') {
            button.onLevelChanged(false, Clock::now());
        } else {
            std::cout << "Unknown. Use p/r/q\n";
        }

        // 处理一次 poll，帮助快速触发超时 click
        button.poll(Clock::now());
    }
}