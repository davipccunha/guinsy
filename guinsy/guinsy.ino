//Librairies
#include <Audio.h>
//#include "MyDsp.h"
#include "FaustGuitar.h"

//Bindings
//Accessory nunchuck1;

//MyDsp myDsp;
FaustGuitar myDsp;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(myDsp,0,out,0);
AudioConnection patchCord1(myDsp,1,out,1);


#define DO 261.63
#define RE 293.66
#define MI 329.63
#define SOL 392.0
#define LA 440.0

void setup() {
	AudioMemory(10);
	Serial.begin(115200);
	audioShield.enable(); // Active la puce SGTL5000
  audioShield.volume(0.6); // Règle un volume de sortie
}

void NotesToPlay(int G,int R, int Y, int B,int O){
	if(G>10){
		Play(DO);
	}

	if(R>10){
		Play(RE);
	}

	if(Y>10){
		Play(MI);
	}

	if(B>10){
		Play(SOL);
	}

	if(O>10){
		Play(LA);
	}

}

void Play(int frequency){
	myDsp.setParamValue("GUITARE_SOLO/GUITARE/frequence", frequency);
	Settings();
	//myDsp.setFreq(frequency);
//	myDsp.setParamValue("/GUITARE_SOLO/GUITARE/pincer_la_corde", 1.0);
delay(10);
//	myDsp.setParamValue("/GUITARE_SOLO/GUITARE/pincer_la_corde", 0.0);
}

void Settings(){
  //float frequency=analogRead(A3);
  //int frequency = map(freq,0,1023,50,2000);
	myDsp.setParamValue("GUITARE_SOLO/CONTROLES/echo", 0.3);
}

void loop() {
	/*
	Serial.println("-------------------------------------------");
	nunchuck1.readData();    // Read inputs and update maps
	nunchuck1.printInputs(); // Print all inputs
	for (int i = 0; i < WII_VALUES_ARRAY_SIZE; i++) {
		Serial.println(
				"Controller Val " + String(i) + " = "
						+ String((uint8_t) nunchuck1.values[i]));
	}
	*/
	//NotesToPlay(nunchuck1.values[11],nunchuck1.values[12],nunchuck1.values[13],nunchuck1.values[14],nunchuck1.values[15]);
	NotesToPlay(25,0,0,0,0);
	delay(2000);
}