#ifndef _channel_led_h
#define _channel_led_h

#include <Arduino.h>
#include <Wire.h>
#include <IS32FL3236A.h>

typedef enum LEDState
{
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_YELLOW
} eLEDState;

class channel_led
{
public:
    channel_led(IS32FL3236A *driver);
    bool begin();
    bool clearAll();
    bool setLEDState(uint8_t channel, eLEDState LED_STATE);
    bool setSleep(bool sleep);
    void setLedBrightness(float brightness);
private:

    IS32FL3236A *_driver;

    float _brightness = 1.0;
};




#endif