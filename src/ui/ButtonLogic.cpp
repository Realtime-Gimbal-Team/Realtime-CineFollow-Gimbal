#include "ButtonLogic.hpp"

ButtonLogic::ButtonLogic(ButtonConfig cfg, Callback cb)
    : cfg_(cfg), cb_(std::move(cb)) {}

void ButtonLogic::onLevelChanged(bool pressed, TimePoint now) {
    // 去抖：变化太快就忽略
    if (last_edge_.time_since_epoch().count() != 0) {
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_edge_);
        if (dt < cfg_.debounce) return;
    }
    last_edge_ = now;

    // 状态没变，不处理
    if (pressed == stable_pressed_) return;

    stable_pressed_ = pressed;

    if (pressed) {
        // 按下
        pressed_at_ = now;
        long_fired_ = false;
    } else {
        // 松开：判断长按/短按
        auto held = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressed_at_);
        if (!long_fired_ && held >= cfg_.long_press_threshold) {
            // 长按（在松开时触发）
            cb_(ButtonEvent::LongPress);
            click_pending_ = false; // 长按不再作为 click 的一部分
            return;
        }

        // 短按：进入 click pending，等待双击窗口
        if (!click_pending_) {
            click_pending_ = true;
            first_click_release_ = now;
        } else {
            auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(now - first_click_release_);
            if (gap <= cfg_.double_click_window) {
                click_pending_ = false;
                cb_(ButtonEvent::DoubleClick);
            } else {
                // 超时了：上一击算 Click，这一击作为新的 pending
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