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
  Serial.begin(9600);
  initializeAudio();
  guitarInput.begin();

  delay(10);
}

void loop() {
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