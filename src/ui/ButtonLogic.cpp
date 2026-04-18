#include "ButtonLogic.hpp"

ButtonLogic::ButtonLogic(ButtonConfig cfg, Callback cb)
    : cfg_(cfg), cb_(std::move(cb)) {}

void ButtonLogic::onLevelChanged(bool pressed, TimePoint now) {

    if (last_edge_.time_since_epoch().count() != 0) {
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_edge_);
        if (dt < cfg_.debounce) return;
    }
    last_edge_ = now;

 
    if (pressed == stable_pressed_) return;

    stable_pressed_ = pressed;

    if (pressed) {

        pressed_at_ = now;
        long_fired_ = false;
    } else {

        auto held = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressed_at_);
        if (!long_fired_ && held >= cfg_.long_press_threshold) {

            cb_(ButtonEvent::LongPress);
            click_pending_ = false; 
            return;
        }


        if (!click_pending_) {
            click_pending_ = true;
            first_click_release_ = now;
        } else {
            auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(now - first_click_release_);
            if (gap <= cfg_.double_click_window) {
                click_pending_ = false;
                cb_(ButtonEvent::DoubleClick);
            } else {

                cb_(ButtonEvent::Click);
                click_pending_ = true;
                first_click_release_ = now;
            }
        }
    }
}

void ButtonLogic::poll(TimePoint now) {
    // 若有 pending click 且超时，则确认单击
    if (click_pending_) {
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - first_click_release_);
        if (dt > cfg_.double_click_window) {
            click_pending_ = false;
            cb_(ButtonEvent::Click);
        }
    }

    // 这里也可以做“按住到点立刻触发长按”（更像真实体验）
    if (stable_pressed_ && !long_fired_) {
        auto held = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressed_at_);
        if (held >= cfg_.long_press_threshold) {
            long_fired_ = true;
            cb_(ButtonEvent::LongPress);
            // 触发长按后，不希望松开再触发 click
            click_pending_ = false;
        }
    }
}