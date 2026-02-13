#pragma once

#include "ControllerMapping.h"
#include "NoteFrequencies.h"

class IntervalMapping : public ControllerMapping {
private:
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

    int currentOctave = 0; // This is an absolute octave, meaning 0 = fundamentals
    int keyIndex = 0;

    bool plusFlag = false, minusFlag = false, joystickPositiveYFlag = false, joystickNegativeYFlag = false;

    void handleOctaveUp();
    void handleOctaveDown();
    void handleKeyUp();
    void handleKeyDown();
    void updateSoundParameters();

public:
    IntervalMapping(GuitarInput& g, FaustGuitar& f)
        : ControllerMapping(g, f)
    {}

    void play() override;
    void handleButtons() override;
};