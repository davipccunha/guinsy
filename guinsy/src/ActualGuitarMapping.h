#pragma once

#include "ControllerMapping.h"

class ActualGuitarMapping : public ControllerMapping {
private:
    int currentOctave = 0; // This is not an absolute octave but rather a relative octave shift that will be applied to the final strings frequency

    bool plusFlag = false, minusFlag = false;

    void handleOctaveUp();
    void handleOctaveDown();
    void updateSoundParameters();

public:
    ActualGuitarMapping(GuitarInput& g, FaustGuitar& f)
        : ControllerMapping(g, f)
    {}

    void play() override;
    void handleButtons() override;
};