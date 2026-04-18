#pragma once
#include "DisplayModel.hpp"

class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual void render(const DisplayModel& m) = 0;
};