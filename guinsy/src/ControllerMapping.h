#pragma once

#include "GuitarInput.h"
#include "FaustGuitar.h"
#include "NoteFrequencies.h"

class ControllerMapping {
protected:
    GuitarInput& guitarInput;
    FaustGuitar& dsp;

    static constexpr float fundamentals[12] = {
        fundamentals::C,
        fundamentals::Cs,
        fundamentals::D,
        fundamentals::Ds,
        fundamentals::E,
        fundamentals::F,
        fundamentals::Fs,
        fundamentals::G,
        fundamentals::Gs,
        fundamentals::A,
        fundamentals::As,
        fundamentals::B
    };

    static constexpr const char* readableKeys[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

public:
    ControllerMapping(GuitarInput& input, FaustGuitar& guitarDsp)
        : guitarInput(input), dsp(guitarDsp)
    {}

    virtual ~ControllerMapping() = default;

    virtual void play() = 0;
    virtual void handleButtons() = 0;
};