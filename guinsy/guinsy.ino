#include <Audio.h>
#include "src/GuitarInput.h"
#include "src/FaustGuitar.h"
#include "src/ActualGuitarMapping.h"
#include "src/IntervalMapping.h"

GuitarInput guitarInput;
FaustGuitar dsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
//AudioConnection patchCord0(dsp, 0, out, 0);
//AudioConnection patchCord1(dsp, 0, out, 1);

AudioOutputUSB usbOut;

AudioConnection patchCord0(dsp, 0, usbOut, 0);
AudioConnection patchCord1(dsp, 1, usbOut, 1);

AudioConnection patchCord2(dsp, 0, out, 0);
AudioConnection patchCord3(dsp, 1, out, 1);


IntervalMapping controllerMapping(guitarInput, dsp);

// Définition des valeurs (puissances de 2)
const byte GREEN_MASK   = 1;  // 00001
const byte RED_MASK  = 2;  // 00010
const byte YELLOW_MASK  = 4;  // 00100
const byte BLUE_MASK   = 8;  // 01000
const byte ORANGE_MASK = 16; // 10000

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

  printPressedFrets();

  play();

  delay(5);
}

void initializeAudio() {
  AudioMemory(64);
  audioShield.enable();
  audioShield.volume(0.25);
}

void OnControlChange(byte channel, byte control, byte value) {
    float valFloat = (float)value;
    
    // Filtres EQ (CC 21 à 26)
    if (control >= 21 && control <= 26) {
        int band = control - 20; // 1 à 6c:\Users\simso\Documents\Gitting\guinsy\guinsy\src\IntervalMapping.cpp
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
void envoyerEtat(byte masqueBoutons, String nomAccord) {
  // Format : N:MASQUE:ACCORD
  // Exemple : N:5:C Maj (5 = Vert + Jaune)
  Serial.print("N:");
  Serial.print(masqueBoutons);
  Serial.print(":");
  Serial.println(nomAccord);
}
void printPressedFrets(){
byte etatActuel = 0;
  if (guitarInput.getGreen()) etatActuel |= GREEN_MASK;
  if (guitarInput.getRed()) etatActuel |= RED_MASK;
  if (guitarInput.getYellow()) etatActuel |= YELLOW_MASK;
  if (guitarInput.getBlue()) etatActuel |= BLUE_MASK;
  if (guitarInput.getOrange()) etatActuel |= ORANGE_MASK;
  envoyerEtat(etatActuel, controllerMapping.getReadableKey());
}

// Debug function
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