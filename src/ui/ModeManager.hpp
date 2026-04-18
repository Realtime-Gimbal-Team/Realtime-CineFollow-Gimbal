#pragma once
#include <functional>
#include <vector>
#include <cstddef>
#include <algorithm>
#include "Mode.hpp"

class ModeManager {
public:
    using Callback = std::function<void(Mode oldMode, Mode newMode)>;

    Mode current() const { return mode_; }
    Mode previous() const { return prev_; }

    
    void set(Mode newMode);

    
    void onButtonEvent(int buttonEvent); 

    std::size_t subscribe(Callback cb);
    void unsubscribe(std::size_t id);

private:
    struct Sub {
        std::size_t id;
        Callback cb;
    };

    void notify(Mode oldMode, Mode newMode);

    Mode mode_{Mode::Idle};
    Mode prev_{Mode::Idle};

    std::vector<Sub> subs_;
    std::size_t nextId_{0};
};