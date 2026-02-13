#include <Audio.h>
#include "src/GuitarInput.h"
#include "src/FaustGuitar.h"
#include "src/ActualGuitarMapping.h"
#include "src/IntervalMapping.h"

GuitarInput guitarInput;
FaustGuitar dsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(dsp, 0, out, 0);
AudioConnection patchCord1(dsp, 0, out, 1);

IntervalMapping controllerMapping(guitarInput, dsp);

void setup() {
  usbMIDI.setHandleControlChange(OnControlChange);
  Serial.begin(9600);
  initializeAudio();
  guitarInput.begin();

  delay(10);
}

void loop() {
  usbMIDI.read(); // Très important pour traiter les messages entrants

  guitarInput.update();

  controllerMapping.handleButtons();

  play();

  delay(5);
}

void initializeAudio() {
  AudioMemory(64);
  audioShield.enable();
  audioShield.volume(0.25);
}
 
 
// Fonction de mapping pour convertir 0-127 en -20dB / +20dB
float mapMIDItoDB(int value) {
    return ((value - 64.0) / 64.0) * 20.0;
}

void OnControlChange(byte channel, byte control, byte value) {
  Serial.printf("MIDI Recu - CC: %d, Valeur: %d\n", control, value);
    float valFloat = (float)value;
    
    // Filtres EQ (CC 21 à 26)
    if (control >= 21 && control <= 26) {
        int band = control - 20; // 1 à 6
        float db = ((valFloat - 64.0) / 64.0) * 20.0;
        String path = "/KISANA_5_STRINGS/EQ/b" + String(band);
        dsp.setParamValue(path.c_str(), db);
    }
    // Master Gain (CC 7)
    else if (control == 7) {
        float db = map(value, 0, 127, -60, 0);
        dsp.setParamValue("/KISANA_5_STRINGS/GLOBAL/master", db);
    }
}

bool strumFlag = false; // This avoids the strum bar to be continuously pressed

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
    controllerMapping.play();
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