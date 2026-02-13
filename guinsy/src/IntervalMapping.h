#pragma once

#include "ControllerMapping.h"
#include "NoteFrequencies.h"

class IntervalMapping : public ControllerMapping {
private:
    int currentOctave = 0; // This is an absolute octave, meaning 0 = fundamentals
    int keyIndex = 0;

    const uint32_t scrollDelay = 120;
    uint32_t lastKeyUp = 0;
    uint32_t lastKeyDown = 0;

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
    const char* getReadableKey();
};