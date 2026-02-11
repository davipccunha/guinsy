#define WII_ADDRESS 0x52

#include <Wire.h>
#include "GuitarInput.h"

GuitarInput::GuitarInput() { }

void GuitarInput::begin() {
    Wire.begin();
    delay(100);
    // Handshake to disable encryption and initialize the controller
    Wire.beginTransmission(WII_ADDRESS);
    Wire.write(0xF0);
    Wire.write(0x55);
    Wire.endTransmission();

    delay(10);

    Wire.beginTransmission(WII_ADDRESS);
    Wire.write(0xFB);
    Wire.write(0x00);
    Wire.endTransmission();

    Serial.println("Wii Guitar Initialized");
}

void GuitarInput::update() {
    // Byte 0: Joystick X (0-63)
    // Byte 1: Joystick Y (0-63)
    // Byte 2: Whammy Bar (0-31)
    // Byte 3: Touch Bar (Slider on some guitars)
    // Byte 4: Frets (Green, Red, Yellow, Blue, Orange)
    // Byte 5: Buttons (Strum, Plus, Minus)

    // Buttons are active LOW (0 means pressed, 1 means released)

    // 1. Tell the guitar we want to read data by sending 0x00
    Wire.beginTransmission(WII_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();

    delay(10);

    // 2. Request 6 bytes from the guitar
    Wire.requestFrom(WII_ADDRESS, 6);

    int i = 0;
    while (Wire.available() && i < 6) {
        data[i] = Wire.read();
        i++;
    }
}

bool GuitarInput::getGreen() {
    return !(data[5] & 0x10);
}

bool GuitarInput::getRed() {
    return !(data[5] & 0x40);
}

bool GuitarInput::getYellow() {
    return !(data[5] & 0x08);
}

bool GuitarInput::getBlue() {
    return !(data[5] & 0x20);
}

bool GuitarInput::getOrange() {
    return !(data[5] & 0x80);
}

bool GuitarInput::getStrumUp() {
    return !(data[5] & 0x01);
}

bool GuitarInput::getStrumDown() {
    return !(data[4] & 0x40);
}

bool GuitarInput::getPlus() {
    return !(data[4] & 0x04);
}

bool GuitarInput::getMinus() {
    return !(data[4] & 0x10);
}

uint8_t GuitarInput::getWhammy() {
    return data[3] & 0x1F; // Range = 14-25
}

std::array<uint8_t, 2> GuitarInput::getJoystick() {
    // Centered at (32, 32)
    uint8_t joyX = data[0] & 0x3F; // Range = 2-61
    uint8_t joyY = data[1] & 0x3F; // Range = 2-61

    return { joyX, joyY };
}