#include "IntervalMapping.h"

void IntervalMapping::play() {
    float frequencies[5];
    float rootFrequency = fundamentals[keyIndex];

    if (guitarInput.getGreen()) {
        frequencies[0] = rootFrequency;
    } else {
        frequencies[0] = 1; // No sound
    }

    if (guitarInput.getRed()) {
        frequencies[1] = rootFrequency * 1.189; // Minor 3rd
    } else {
        frequencies[1] = 1; // No sound
    }

    if (guitarInput.getYellow()) {
        frequencies[2] = rootFrequency * 1.259; // Major 3rd
    } else {
        frequencies[2] = 1; // No sound
    }

    if (guitarInput.getBlue()) {
        frequencies[3] = rootFrequency * 1.414; // Diminished 5th //1.334; // 4th
    } else {
        frequencies[3] = 1; // No sound
    }

    if (guitarInput.getOrange()) {
        frequencies[4] = rootFrequency * 1.498; // 5th
    } else {
        frequencies[4] = 1; // No sound
    }

    float currentOctaveFactor = pow(2.0, this->currentOctave);

    dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/frequence", frequencies[0] * currentOctaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/frequence", frequencies[1] * currentOctaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/frequence", frequencies[2] * currentOctaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/frequence", frequencies[3] * currentOctaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/frequence", frequencies[4] * currentOctaveFactor);

    //Pinch
    if (frequencies[0] != 1) {
        dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/pincer", 1.0);	
    }
    if (frequencies[1] != 1) {
        dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/pincer", 1.0);
    }
    if (frequencies[2] != 1) {
        dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/pincer", 1.0);
    }
    if (frequencies[3] != 1) {
        dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/pincer", 1.0);
    }
    if (frequencies[4] != 1) {
        dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/pincer", 1.0);
    }

    delay(10);

    dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/pincer", 0.0);
}

void IntervalMapping::handleButtons() {
    this->handleOctaveUp();
    this->handleOctaveDown();
    this->handleKeyUp();
    this->handleKeyDown();
    this->updateSoundParameters();
}

void IntervalMapping::handleOctaveUp() {
    if (!guitarInput.getPlus() && !plusFlag) {
        return;
    }

    if (guitarInput.getPlus() && plusFlag) {
        return;
    }

    if (!guitarInput.getPlus() && plusFlag) {
        plusFlag = false;
        return;
    };

    if (guitarInput.getPlus() && !plusFlag) {
        plusFlag = true;
        currentOctave = min(currentOctave + 1, 5); // For some reason, a higher octave does not change anything on the headphones
        Serial.printf("Current key: %s\n", getReadableKey());
    }
}

void IntervalMapping::handleOctaveDown() {
    if (!guitarInput.getMinus() && !minusFlag) {
        return;
    }

    if (guitarInput.getMinus() && minusFlag) {
        return;
    }

    if (!guitarInput.getMinus() && minusFlag) {
        minusFlag = false;
        return;
    };

    if (guitarInput.getMinus() && !minusFlag) {
        minusFlag = true;
        currentOctave = max(0, currentOctave - 1);
        Serial.printf("Current key: %s\n", getReadableKey());
    }
}

void IntervalMapping::handleKeyUp() {
    bool joystickUp = guitarInput.getJoystick()[1] < 15;

    if (joystickUp) {
        uint32_t currentTime = millis();

        if (!joystickNegativeYFlag || (currentTime - lastKeyUp >= scrollDelay)) {
            keyIndex = (keyIndex + 1) % 12;
            
            Serial.printf("Current key: %s\n", getReadableKey());

            lastKeyUp = currentTime;
            joystickNegativeYFlag = true;
        }
    } else {
        joystickNegativeYFlag = false;
    }
}

void IntervalMapping::handleKeyDown() {
    bool joystickDown = guitarInput.getJoystick()[1] > 50;

    if (joystickDown) {
        uint32_t currentTime = millis();

        if (!joystickPositiveYFlag || (currentTime - lastKeyDown >= scrollDelay)) {
            keyIndex = (12 + keyIndex - 1) % 12; // If index is negative, we go back. For index = 0, index - 1 = -1, -1 + 12 = 11
            Serial.printf("Current key: %s\n", getReadableKey());

            lastKeyDown = currentTime;
            joystickPositiveYFlag = true;
        }
    } else {
        joystickPositiveYFlag = false;
    }
}

void IntervalMapping::updateSoundParameters() {
	// Echo
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/echo", 0.0);

    // Pitch shift
    uint8_t whammy = guitarInput.getWhammy();
    if (whammy < 16) return; // The whammy bar is a little bit unstable at rest

    int pitchShift = map(whammy, 16, 25, 0, 2);
    dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/pitch_shift", (float) pitchShift); 

	// Timbre
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/timbre", 0.5);
}

const char* IntervalMapping::getReadableKey() {
    static char buffer[16]; 

    // snprintf is the C++ version of the "f-string"
    snprintf(buffer, sizeof(buffer), "%s%d", readableKeys[keyIndex], currentOctave);

    return buffer;
}