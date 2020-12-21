#include "Particle.h"

#ifndef rgb_controller_h
#define rgb_controller_h

class RgbController
{
private:
    u8_t controlledBreath = 0;
    u16_t activeMutilple = 0;
    const u8_t MAX_BRIGHT = 170;
    const u8_t STEP = 1;
    const u8_t PANIC_STEP = MAX_BRIGHT;
    const u16_t MULTIPLIED_CONNTECTED = MAX_BRIGHT;
    const u16_t MULTIPLIED_PANICKED = MAX_BRIGHT * 8;

    const u8_t MID_BRIGHT = 127;
    const u8_t RAPID_FLASH_COUNT = 30;
    const u8_t RAPID_FLASH_DELAY = 50;
    const u8_t OFF = 0;
    u16_t multiple = 0;
    u8_t step = 0;
    bool adder = true;
    void routine();
    bool foundConnection = false;
    bool lostConnection = false;
    bool trashConnection = false;
    void setThrash();
    void setConnected();
    void setPanic();
    u8_t CONNECTED_COLOR[3] = {6, 196, 222};
    u8_t PANIC_COLOR[3] = {226, 76, 76};
    u8_t THRASH_COLOR[3] = {255, 170, 49};
    const u8_t RED = 0;
    const u8_t GREEN = 1;
    const u8_t BLUE = 2;

public:
    ~RgbController();
    RgbController();
    void control(bool control);
    void breath();
    void panic();
    void thrash();
    void rapidFlash();
    void delayBlink(size_t durration, u8_t color);
    void color(u8_t red, u8_t green, u8_t blue);
    // constexpr static u8_t PANIC_COLOR[] = {226, 76, 76};
    // constexpr static u8_t CONNECTED_COLOR[] = {6, 196, 222};
    // constexpr static u8_t THRASH_COLOR[] = {255, 170, 49};
    const static u8_t CONNECTED = 0;
    const static u8_t THRASH = 1;
    const static u8_t PANIC = 2;
};

#endif