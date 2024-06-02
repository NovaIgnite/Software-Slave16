#include <channel_led.h>

/*
This class controls the state of the channel leds, it provides functions for clearing all the leds, setting the color, brightness and sleep functionality
*/

channel_led::channel_led(IS32FL3236A *driver)
{
    _driver = driver;
}
bool channel_led::begin()
{
    bool error = 1;
    if (_driver->begin() == 1)
    {
        error = _driver->sleep(0);
        error = _driver->setFrequency(1);
        error = _driver->clear();
        error = _driver->update();
        error = clearAll();
    }
    return error;
}
bool channel_led::clearAll()
{
    bool error = 1;
    for (int i = 0; i < 16; i++)
    {
        error = setLEDState(i, LED_OFF);
    }
    error = _driver->update();
    return error;
}
bool channel_led::setLEDState(uint8_t channel, eLEDState LED_STATE)
{
    bool error = 1;
    if (channel >= 0 && channel <= 15)
    {
        uint8_t led_channel_r = channel * 2 + 1;
        uint8_t led_channel_g = channel * 2;

        switch (LED_STATE)
        {
        case LED_OFF:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_r, 0);
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_g, 0);
            break;
        case LED_RED:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_r, _driver->gamma64(roundf(_brightness * 63)));
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_g, 0);
            break;
        case LED_GREEN:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_r, 0);
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_g, _driver->gamma64(roundf(_brightness * 63)));
            break;
        case LED_YELLOW:
            error = _driver->setLedParam(led_channel_r, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_r, _driver->gamma64(roundf(_brightness * 63)));
            error = _driver->setLedParam(led_channel_g, IS32FL3236A_IMAX, 1);
            error = _driver->setLedPwm(led_channel_g, _driver->gamma64(roundf(_brightness * 32)));
            break;
        default:
            break;
        }
        error = _driver->update();
    }
    return error;
}
bool channel_led::setSleep(bool sleep)
{
    return _driver->sleep(!sleep);
}
void channel_led::setLedBrightness(float brightness)
{
    if (brightness >= 0.0 && brightness <= 1.0005)
    {
        _brightness = brightness;
    }
}
