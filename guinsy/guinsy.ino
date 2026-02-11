#include <Audio.h>
#include "src/GuitarInput.h"
#include "src/MyDsp.h"
#include "src/NoteFrequencies.h"

GuitarInput guitarInput;
MyDsp sine;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(sine, 0, out, 0);
AudioConnection patchCord1(sine, 0, out, 1);

void setup() {
  Serial.begin(9600);
  initializeAudio();
  guitarInput.begin();
}

void loop() {
  guitarInput.update();
  handleGuitarInput();
  printGuitarInput();

  delay(10);
}

void initializeAudio() {
  AudioMemory(2);
  audioShield.enable();
  audioShield.volume(0.25);
}

void handleGuitarInput() {
  if (!guitarInput.isAnyFretPressed()) {
    sine.setFreq(1.0f);
  }

  if (guitarInput.getGreen()) {
    sine.setFreq(notes::E2);
  }

  if (guitarInput.getRed()) {
    sine.setFreq(notes::A2);
  }

  if (guitarInput.getYellow()) {
    sine.setFreq(notes::D3);
  }

  if (guitarInput.getBlue()) {
    sine.setFreq(notes::G3);
  }

  if (guitarInput.getOrange()) {
    sine.setFreq(notes::B3);
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