#include <channel_led.h>

/*
This class controls the state of the channel leds, it provides functions for clearing all the leds, setting the color, brightness and sleep functionality
*/

channel_led::channel_led(IS32FL3236A *driver)
{
    _driver = driver; // make the variable neat
}
bool channel_led::begin() // function for starting up the driver
{
    bool error = 1;            // error flag
    if (_driver->begin() == 1) // start the driver
    {
        error = _driver->sleep(0);        // disable driver
        error = _driver->setFrequency(1); // set PWM Frequency to 25kHz
        error = _driver->clear();         // clear all LEDs
        error = _driver->update();        // update the LEDs
        error = clearAll();               // clear them again to be sure
    }
    return error; // return 0 or 1 as an error
}
bool channel_led::clearAll() // clear all the channels
{
    bool error = 1;              // error flag
    for (int i = 0; i < 16; i++) // loop through all channels
    {
        error = setLEDState(i, LED_OFF); // set LEDs to off
    }
    error = _driver->update(); // update
    return error;              // return 0 or 1 as an error
}
bool channel_led::setLEDState(uint8_t channel, eLEDState LED_STATE) // set the specified LED channel to the specified state
{
    bool error = 1; // error flag
    if (channel >= 0 && channel <= 15)
    {
        uint8_t led_channel_r = channel * 2 + 1;
        uint8_t led_channel_g = channel * 2;

        switch (LED_STATE)
        {
        case LED_OFF:                                                         // set to OFF
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1); // param for LEDs
            error = _driver->setLedPwm(led_channel_r, 0);                     // set to 0 (OFF)
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1); // param for LEDs
            error = _driver->setLedPwm(led_channel_g, 0);                     // set to 0 (OFF)
            break;
        case LED_RED:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_r, _driver->gamma64(roundf(_brightness * 63))); // linearize brightness for human eye and set, brightness multiplied then rounded.
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_g, 0);                                          // set to 0 (OFF)
            break;
        case LED_GREEN:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_r, 0);                                          // set to 0 (OFF)
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_g, _driver->gamma64(roundf(_brightness * 63))); // linearize brightness for human eye and set, brightness multiplied then rounded.
            break;
        case LED_YELLOW:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_r, _driver->gamma64(roundf(_brightness * 63))); // linearize brightness for human eye and set, brightness multiplied then rounded.
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);                      // param for LEDs
            error = _driver->setLedPwm(led_channel_g, _driver->gamma64(roundf(_brightness * 32))); // linearize brightness for human eye and set, brightness multiplied then rounded. (half brightness for mixing yellow)
            break;
        default:
            break;
        }
        error = _driver->update(); // update
    }
    return error; // return 0 or 1 as an error
}
bool channel_led::setSleep(bool sleep)
{
    return _driver->sleep(!sleep); // set sleep and return error
}
void channel_led::setLedBrightness(float brightness)
{
    if (brightness >= 0.0 && brightness <= 1.0005) // check if brightness is in range
    {
        _brightness = brightness; // set brightness
    }
}
