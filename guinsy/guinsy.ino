#include <Wire.h>
#include "lib/NotesFrequencies/NoteFrequencies.h"

#define WII_ADDRESS 0x52

void setup() {
  Serial.begin(115200);
  Wire.begin(); // On Teensy 4.0, this uses Pin 18 (SDA) and Pin 19 (SCL)
  
  // Handshake to disable encryption and initialize the controller
  // This is the "modern" way to initialize Wii extensions
  delay(100);
  Wire.beginTransmission(WII_ADDRESS);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  delay(10);
  
  Wire.beginTransmission(WII_ADDRESS);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(20);

  Serial.println("Wii Guitar Initialized");
}

void loop() {
  // 1. Tell the guitar we want to read data by sending 0x00
  Wire.beginTransmission(WII_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  
  delayMicroseconds(200); // Small delay for the guitar to prepare data

  // 2. Request 6 bytes from the guitar
  Wire.requestFrom(WII_ADDRESS, 6);
  
  uint8_t buffer[6];
  int i = 0;
  while (Wire.available() && i < 6) {
    buffer[i] = Wire.read();
    i++;
  }

  if (i == 6) {
    parseGuitarData(buffer);
  }

  delay(10); // Adjust for desired sampling rate (10ms = 100Hz)
}

void parseGuitarData(uint8_t* data) {
  // Byte 0: Joystick X (0-63)
  // Byte 1: Joystick Y (0-63)
  // Byte 2: Whammy Bar (0-31)
  // Byte 3: Touch Bar (Slider on some guitars)
  // Byte 4: Buttons (Green, Red, Yellow, Blue, Orange)
  // Byte 5: Buttons (Strum, Plus, Minus)

  // Buttons are active LOW (0 means pressed, 1 means released)
  // We use "!" and bitwise AND to make them "true" when pressed.

  bool green  = !(data[5] & 0x10);
  bool red    = !(data[5] & 0x40);
  bool yellow = !(data[5] & 0x08);
  bool blue   = !(data[5] & 0x20);
  bool orange = !(data[5] & 0x80);
  
  bool strumUp   = !(data[5] & 0x01);
  bool strumDown = !(data[4] & 0x40);
  
  bool plus  = !(data[4] & 0x04);
  bool minus = !(data[4] & 0x10);

  int whammy = data[3] & 0x1F; // 5-bit value

  // Debug Output
  if (green) Serial.print("G ");
  if (red)   Serial.print("R ");
  if (yellow)Serial.print("Y ");
  if (blue)  Serial.print("B ");
  if (orange)Serial.print("O ");
  
  if (strumUp)   Serial.print("STRUM_UP ");
  if (strumDown) Serial.print("STRUM_DOWN ");
  
  Serial.print("Whammy: ");
  Serial.println(whammy);
}