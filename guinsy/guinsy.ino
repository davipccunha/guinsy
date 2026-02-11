#include <Audio.h>
#include "src/GuitarInput.h"
#include "src/NoteFrequencies.h"
#include "src/FaustGuitar.h"

GuitarInput guitarInput;
FaustGuitar dsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(dsp, 0, out, 0);
AudioConnection patchCord1(dsp, 1, out, 1);

void setup() {
  Serial.begin(9600);
  initializeAudio();
  guitarInput.begin();
}

void loop() {
  guitarInput.update();
  settings();
	play();

  // handleGuitarInput();
  // printGuitarInput();

  delay(10);
}

void initializeAudio() {
  AudioMemory(2);
  audioShield.enable();
  audioShield.volume(0.25);
}
 
void play(){
  float frequencies[5];

  if (guitarInput.getGreen()) { 
    frequencies[0] = notes::E2;
  } else {
    frequencies[0] = 1.0f;
  }

  if (guitarInput.getRed()) {
    frequencies[1] = notes::A3;
  } else {
    frequencies[1] = 1.0f;
  }

  if (guitarInput.getYellow()) {
    frequencies[2] = notes::D3;
  } else {
    frequencies[2] = 1.0f;
  }

  if (guitarInput.getBlue()) {
    frequencies[3] = notes::G3;
  } else {
    frequencies[3] = 1.0f;
  }

  if (guitarInput.getOrange()) {
    frequencies[4] = notes::B3;
  } else {
    frequencies[4] = 1.0f;
  }

	dsp.setParamValue("/KISANA_GUITAR/Corde_1/frequence", frequencies[0]);
	dsp.setParamValue("/KISANA_GUITAR/Corde_2/frequence", frequencies[1]);
	dsp.setParamValue("/KISANA_GUITAR/Corde_3/frequence", frequencies[2]);
	dsp.setParamValue("/KISANA_GUITAR/Corde_4/frequence", frequencies[3]);
	dsp.setParamValue("/KISANA_GUITAR/Corde_5/frequence", frequencies[4]);
 
	//Pinch
	if(frequencies[0] < 20){
		dsp.setParamValue("/KISANA_GUITAR/Corde_1/pincer", 1.0);	
	}
	if(frequencies[1] < 20){
		dsp.setParamValue("/KISANA_GUITAR/Corde_2/pincer", 1.0);
	}
	if(frequencies[2] < 20){
		dsp.setParamValue("/KISANA_GUITAR/Corde_3/pincer", 1.0);
	}
	if(frequencies[3] < 20){
		dsp.setParamValue("/KISANA_GUITAR/Corde_4/pincer", 1.0);
	}
	if(frequencies[4] < 20){
		dsp.setParamValue("/KISANA_GUITAR/Corde_5/pincer", 1.0);
	}
	delay(10);
	dsp.setParamValue("/KISANA_GUITAR/Corde_1/pincer", 0.0);
	dsp.setParamValue("/KISANA_GUITAR/Corde_2/pincer", 0.0);
	dsp.setParamValue("/KISANA_GUITAR/Corde_3/pincer", 0.0);
	dsp.setParamValue("/KISANA_GUITAR/Corde_4/pincer", 0.0);
	dsp.setParamValue("/KISANA_GUITAR/Corde_5/pincer", 0.0);
}
 
void settings(){
	// Echo
	dsp.setParamValue("/KISANA_GUITAR/GLOBAL/echo", 0.0);
 
	// float echoValue = (analogRead(A3) / 1023.0) * 0.9;
	// dsp.setParamValue("/KISANA_GUITAR/GLOBAL/echo", echoValue);
 
 
	// Timbre
	dsp.setParamValue("/KISANA_GUITAR/GLOBAL/timbre", 0.5);
 
	// float timbreValue = (analogRead(A8) / 1023.0) * 0.9;
	// dsp.setParamValue("/KISANA_GUITAR/GLOBAL/timbre", timbreValue);
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