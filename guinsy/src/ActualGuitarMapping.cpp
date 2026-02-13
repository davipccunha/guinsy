#include "ActualGuitarMapping.h"
#include "NoteFrequencies.h"

void ActualGuitarMapping::play() {
    float frequencies[5];

    if (guitarInput.getGreen()) {
        frequencies[0] = notes::E2;
    } else {
        frequencies[0] = 1; // No sound
    }

    if (guitarInput.getRed()) {
        frequencies[1] = notes::A2;
    } else {
        frequencies[1] = 1; // No sound
    }

    if (guitarInput.getYellow()) {
        frequencies[2] = notes::D3;
    } else {
        frequencies[2] = 1; // No sound
    }

    if (guitarInput.getBlue()) {
        frequencies[3] = notes::G3;
    } else {
        frequencies[3] = 1; // No sound
    }

    if (guitarInput.getOrange()) {
        frequencies[4]=notes::B3;
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

void ActualGuitarMapping::handleButtons() {
    this->handleOctaveUp();
    this->handleOctaveDown();
    this->updateSoundParameters();
}

void ActualGuitarMapping::handleOctaveUp() {
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
        currentOctave = min(currentOctave + 1, 3); // For some reason, a higher octave then 4 does not change anything on the headphones
        Serial.printf("Current octave: %d\n", currentOctave);
    }
}

void ActualGuitarMapping::handleOctaveDown() {
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
        Serial.printf("Current octave: %d\n", currentOctave);
    }
}

void ActualGuitarMapping::updateSoundParameters() {
	// Echo
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/echo", 0.0);

    // Pitch shift
    uint8_t whammy = guitarInput.getWhammy();
    int pitchShift = map(whammy, 14, 25, 0, 12);
    dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/pitch_shift", (float) pitchShift); 

	// Timbre
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/timbre", 0.5);
}