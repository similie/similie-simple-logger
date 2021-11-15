#include "rgb_controller.h"

RgbController::RgbController()
{
}

RgbController::~RgbController()
{
}

void RgbController::control(bool control)
{
    foundConnection = false;
    lostConnection = false;
    trashConnection = false;
    RGB.control(control);
}

void RgbController::color(uint8_t red, uint8_t green, uint8_t blue)
{
    RGB.color(red, green, blue);
}

void RgbController::rapidFlash()
{

    bool on = false;
    uint8_t i = 0;
    while (i < RAPID_FLASH_COUNT)
    {
        if (on)
        {
            RGB.brightness(MID_BRIGHT);
        }
        else
        {
            RGB.brightness(OFF);
        }

        i++;
        on = !on;
        delay(RAPID_FLASH_DELAY);
    }

    RGB.brightness(OFF);
}

void RgbController::setColor(uint8_t color)
{
    control(true);
    switch (color)
    {
    case RgbController::CONNECTED:
        this->color(CONNECTED_COLOR[RED], CONNECTED_COLOR[GREEN], CONNECTED_COLOR[BLUE]);
        break;
    case RgbController::THRASH:
        this->color(THRASH_COLOR[RED], THRASH_COLOR[GREEN], THRASH_COLOR[BLUE]);
        break;
    case RgbController::PANIC:
        this->color(PANIC_COLOR[RED], PANIC_COLOR[GREEN], PANIC_COLOR[BLUE]);
        break;
    }
}

void RgbController::delayBlink(size_t durration, uint8_t color)
{
    control(true);
    bool on = true;
    switch (color)
    {
    case RgbController::CONNECTED:
        this->color(CONNECTED_COLOR[RED], CONNECTED_COLOR[GREEN], CONNECTED_COLOR[BLUE]);
        break;
    case RgbController::THRASH:
        this->color(THRASH_COLOR[RED], THRASH_COLOR[GREEN], THRASH_COLOR[BLUE]);
        break;
    case RgbController::PANIC:
        this->color(PANIC_COLOR[RED], PANIC_COLOR[GREEN], PANIC_COLOR[BLUE]);
        break;
    }

    for (size_t i = 0; i < durration; i++)
    {
        uint8_t status = on ? MAX_BRIGHT : OFF;
        on = !on;
        RGB.brightness(status);
        delay(1000);
    }
}

void RgbController::setConnected()
{
    // take control of the RGB LED
    control(true);

    color(CONNECTED_COLOR[RED], CONNECTED_COLOR[GREEN], CONNECTED_COLOR[BLUE]);
    activeMutilple = MULTIPLIED_CONNTECTED;
    foundConnection = true;
    lostConnection = false;
    trashConnection = false;
    rapidFlash();
    step = STEP;
}

void RgbController::setPanic()
{
    // take control of the RGB LED
    control(true);
    color(PANIC_COLOR[RED], PANIC_COLOR[GREEN], PANIC_COLOR[BLUE]);
    // color(226, 76, 76);
    activeMutilple = MULTIPLIED_PANICKED;
    foundConnection = false;
    lostConnection = true;
    trashConnection = false;
    step = PANIC_STEP;
}

void RgbController::setThrash()
{
    // take control of the RGB LED
    control(true);
    // color(255, 170, 49);
    color(THRASH_COLOR[RED], THRASH_COLOR[GREEN], THRASH_COLOR[BLUE]);
    activeMutilple = MULTIPLIED_PANICKED;
    foundConnection = false;
    lostConnection = false;
    trashConnection = true;
    step = PANIC_STEP;
}

void RgbController::routine()
{
    if (multiple <= activeMutilple)
    {
        multiple++;
        return;
    }
    else
    {
        multiple = 0;
    }

    if (controlledBreath <= 0)
    {
        adder = true;
    }
    else if (controlledBreath >= MAX_BRIGHT)
    {
        adder = false;
    }

    if (adder)
    {

        controlledBreath += step;
    }
    else
    {
        controlledBreath -= step;
    }
    RGB.brightness(controlledBreath);
}

void RgbController::breath()
{

    if (!foundConnection)
    {
        setConnected();
    }

    routine();
}

void RgbController::panic()
{
    if (!lostConnection)
    {
        setPanic();
    }

    routine();
}

void RgbController::thrash()
{
    if (!trashConnection)
    {
        setThrash();
    }

    routine();
}
