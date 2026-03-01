#include "ButtonEvent.hpp"
#include "Mode.hpp"

class ModeController {
public:
    Mode current() const { 
        return mode_; 
    }

    void onButtonEvent(ButtonEvent e) {
        switch (e) {
            case ButtonEvent::Click:
                if (mode_ == Mode::Follow) mode_ = Mode::Lock;
                else if (mode_ == Mode::Lock) mode_ = Mode::Follow;
                else mode_ = Mode::Follow;
                break;

            case ButtonEvent::LongPress:
                mode_ = Mode::Idle;
                break;

            case ButtonEvent::DoubleClick: {
                Mode before = mode_;
                mode_ = Mode::Reset;        
                mode_ = before;            
                break;
            }
        }
    }

private:
    Mode mode_{Mode::Idle};
};