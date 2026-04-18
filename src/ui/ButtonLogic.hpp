#pragma once
#include <chrono>
#include <functional>
#include "ButtonEvent.hpp"

struct ButtonConfig {
    std::chrono::milliseconds debounce{30};
    std::chrono::milliseconds double_click_window{500};
    std::chrono::milliseconds long_press_threshold{1000};
};

class ButtonLogic {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Callback = std::function<void(ButtonEvent)>;

    ButtonLogic(ButtonConfig cfg, Callback cb);


    void onLevelChanged(bool pressed, TimePoint now);


    void poll(TimePoint now);

private:
    ButtonConfig cfg_;
    Callback cb_;

    bool stable_pressed_{false};  
    TimePoint last_edge_{};
    TimePoint pressed_at_{};

    bool click_pending_{false};
    TimePoint first_click_release_{};
    bool long_fired_{false};
};