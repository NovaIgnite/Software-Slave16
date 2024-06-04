#ifndef _channel_led_h
#define _channel_led_h

#include <Arduino.h>
#include <Wire.h>
#include <IS32FL3236A.h>
// include all the necessary libraries

typedef enum LEDState
{
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_YELLOW
} eLEDState; // This typedef hold all the possible led states

class channel_led // class
{
public:
    channel_led(IS32FL3236A *driver);                       // constructor with the driver libary
    bool begin();                                           // startup libary
    bool clearAll();                                        // set all the led's to off
    bool setLEDState(uint8_t channel, eLEDState LED_STATE); // set the corrsponding channel to a specified state (RED, GREEN, YELLOW or OFF)
    bool setSleep(bool sleep);                              // activate the sleep mode
    void setLedBrightness(float brightness);                // set the brightness of the led driver between 0.0 and 1.0
private:
    IS32FL3236A *_driver;    // led driver pointer
    float _brightness = 1.0; // brightness
};

#endif