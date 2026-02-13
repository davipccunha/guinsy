#pragma once

#include "ControllerMapping.h"
#include "NoteFrequencies.h"

class IntervalMapping : public ControllerMapping {
private:
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
    const char* getReadableKey();
};