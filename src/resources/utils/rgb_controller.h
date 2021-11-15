#include "Particle.h"

#ifndef rgb_controller_h
#define rgb_controller_h

class RgbController
{
private:
    uint8_t controlledBreath = 0;
    u16_t activeMutilple = 0;
    const uint8_t MAX_BRIGHT = 170;
    const uint8_t STEP = 1;
    const uint8_t PANIC_STEP = MAX_BRIGHT;
    const u16_t MULTIPLIED_CONNTECTED = MAX_BRIGHT;
    const u16_t MULTIPLIED_PANICKED = MAX_BRIGHT * 8;

    const uint8_t MID_BRIGHT = 127;
    const uint8_t RAPID_FLASH_COUNT = 30;
    const uint8_t RAPID_FLASH_DELAY = 50;
    const uint8_t OFF = 0;
    u16_t multiple = 0;
    uint8_t step = 0;
    bool adder = true;
    void routine();
    bool foundConnection = false;
    bool lostConnection = false;
    bool trashConnection = false;
    void setThrash();
    void setConnected();
    void setPanic();
    uint8_t CONNECTED_COLOR[3] = {6, 196, 222};
    uint8_t PANIC_COLOR[3] = {226, 76, 76};
    uint8_t THRASH_COLOR[3] = {255, 170, 49};
    const uint8_t RED = 0;
    const uint8_t GREEN = 1;
    const uint8_t BLUE = 2;

public:
    ~RgbController();
    RgbController();
    void control(bool control);
    void breath();
    void panic();
    void thrash();
    void rapidFlash();
    void delayBlink(size_t durration, uint8_t color);
    void color(uint8_t red, uint8_t green, uint8_t blue);
    // constexpr static uint8_t PANIC_COLOR[] = {226, 76, 76};
    // constexpr static uint8_t CONNECTED_COLOR[] = {6, 196, 222};
    // constexpr static uint8_t THRASH_COLOR[] = {255, 170, 49};
    const static uint8_t CONNECTED = 0;
    const static uint8_t THRASH = 1;
    const static uint8_t PANIC = 2;
    void setColor(uint8_t color);
};

#endif