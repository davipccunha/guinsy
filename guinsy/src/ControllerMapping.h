#pragma once

#include "GuitarInput.h"
#include "FaustGuitar.h"

class ControllerMapping {
protected:
    GuitarInput& guitarInput;
    FaustGuitar& dsp;

public:
    ControllerMapping(GuitarInput& input, FaustGuitar& guitarDsp)
        : guitarInput(input), dsp(guitarDsp)
    {}

    virtual ~ControllerMapping() = default;

    virtual void play() = 0;
    virtual void handleButtons() = 0;
};