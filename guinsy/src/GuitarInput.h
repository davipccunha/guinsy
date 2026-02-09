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

    std::array<uint8_t, 2> getJoystick();
    uint8_t getWhammy();

private:
    uint8_t data[6];
};