#include <Audio.h>
#include "src/GuitarInput.h"
#include "src/NoteFrequencies.h"
#include "src/FaustGuitar.h"

GuitarInput guitarInput;
FaustGuitar dsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(dsp, 0, out, 0);
AudioConnection patchCord1(dsp, 0, out, 1);

void setup() {
  Serial.begin(9600);
  initializeAudio();
  guitarInput.begin();

  delay(10);
}

void loop() {
  guitarInput.update();
  updateSoundParameters();

  play();

  delay(5);
}

void initializeAudio() {
  AudioMemory(64);
  audioShield.enable();
  audioShield.volume(0.25);
}
 
bool strumFlag = false; // This avoids the strum bar to be continuously pressed
int currentOctave = 0; // This is not an absolute octave but rather a relative octave shift that will be applied to the final strings frequency

void play() {
  if (!guitarInput.getStrum() && !strumFlag) {
    return;
  }

  if (guitarInput.getStrum() && strumFlag) {
    return;
  }

  if (!guitarInput.getStrum() && strumFlag) {
    strumFlag = false;
    return;
  };

  if (guitarInput.getStrum() && !strumFlag) {
    strumFlag = true;
  
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

    int octaveFactor = pow(2, currentOctave);

    dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/frequence", frequencies[0] * octaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/frequence", frequencies[1] * octaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/frequence", frequencies[2] * octaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/frequence", frequencies[3] * octaveFactor);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/frequence", frequencies[4] * octaveFactor);
  
    //Pinch
    if(frequencies[0] != 1){
      dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/pincer", 1.0);	
    }
    if(frequencies[1] != 1){
      dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/pincer", 1.0);
    }
    if(frequencies[2] != 1){
      dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/pincer", 1.0);
    }
    if(frequencies[3] != 1){
      dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/pincer", 1.0);
    }
    if(frequencies[4] != 1){
      dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/pincer", 1.0);
    }

    delay(10);

    dsp.setParamValue("/KISANA_5_STRINGS/Corde_1/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_2/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_3/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_4/pincer", 0.0);
    dsp.setParamValue("/KISANA_5_STRINGS/Corde_5/pincer", 0.0);
  }
}

bool plusFlag = false, minusFlag = false;

void updateSoundParameters() {
 
	// Echo
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/echo", 0.0);

  // Pitch shift
  uint8_t whammy = guitarInput.getWhammy();
  int pitchShift = map(whammy, 14, 25, 0, 12);
  dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/pitch_shift", (float) pitchShift); 
 
	// Timbre
	dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/timbre", 0.5);

  // Octave
  handleOctaveUp();
  handleOctaveDown();
}

void handleOctaveUp() {
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
    currentOctave = min(currentOctave + 1, 4); // For some reason, a higher octave then 4 does not change anything on the headphones
    Serial.printf("Current octave: %d\n", currentOctave);
  }
}

void handleOctaveDown() {
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

void printGuitarInput() {
  if (guitarInput.getGreen()) Serial.print("G ");
  if (guitarInput.getRed()) Serial.print("R ");
  if (guitarInput.getYellow()) Serial.print("Y ");
  if (guitarInput.getBlue()) Serial.print("B ");
  if (guitarInput.getOrange()) Serial.print("O ");
  
  if (guitarInput.getStrumUp()) Serial.print("STRUM_UP ");
  if (guitarInput.getStrumDown()) Serial.print("STRUM_DOWN ");

  if (guitarInput.getPlus()) Serial.print("+ ");
  if (guitarInput.getMinus()) Serial.print("- ");

  Serial.printf("Whammy: %d\n", guitarInput.getWhammy());

  std::array<uint8_t, 2> joystick = guitarInput.getJoystick();

  Serial.printf("Joystick: (%d, %d)\n", joystick[0], joystick[1]);
  Serial.println();
}