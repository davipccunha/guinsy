#pragma once
#include <cstdint>

class GuitarInput {
public:
    GuitarInput();

    void begin();
    void update();

    bool getGreen();
    bool getRed();
    bool getYellow();
    bool getBlue();
    bool getOrange();

    bool getStrumUp();
    bool getStrumDown();

    bool getPlus();
    bool getMinus();

    uint8_t getWhammy();
    std::array<uint8_t, 2> getJoystick();

    bool isAnyFretPressed();

private:
    uint8_t data[6];
};