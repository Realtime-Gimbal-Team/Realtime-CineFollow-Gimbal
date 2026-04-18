#include "ModeManager.hpp"
#include "ButtonEvent.hpp"

void ModeManager::set(Mode newMode) {
    if (newMode == mode_) return;
    Mode old = mode_;
    prev_ = mode_;
    mode_ = newMode;
    notify(old, newMode);
}

std::size_t ModeManager::subscribe(Callback cb) {
    const std::size_t id = ++nextId_;
    subs_.push_back(Sub{id, std::move(cb)});
    return id;
}

void ModeManager::unsubscribe(std::size_t id) {
    subs_.erase(std::remove_if(subs_.begin(), subs_.end(),
                               [&](const Sub& s){ return s.id == id; }),
                subs_.end());
}

void ModeManager::notify(Mode oldMode, Mode newMode) {
    for (auto& s : subs_) {
        if (s.cb) s.cb(oldMode, newMode);
    }
}


void ModeManager::onButtonEvent(int e) {
    ButtonEvent ev = static_cast<ButtonEvent>(e);

    switch (ev) {
        case ButtonEvent::Click:
            if (mode_ == Mode::Follow) set(Mode::Lock);
            else if (mode_ == Mode::Lock) set(Mode::Follow);
            else set(Mode::Follow);
            break;

        case ButtonEvent::LongPress:
            set(Mode::Idle);
            break;

        case ButtonEvent::DoubleClick: {
            Mode before = mode_;
            set(Mode::Reset);

            set(before == Mode::Reset ? Mode::Idle : before);
            break;
        }
    }
}