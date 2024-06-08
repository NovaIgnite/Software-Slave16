#ifndef _resistance_control16_h
#define _resistance_control16_h

#include <Arduino.h>
#include <channel_led.h>
#include <neotimer.h>
// include all libraries

class resistance_control // class
{
public:
    resistance_control(channel_led *led_channel, uint8_t pins[16], volatile uint16_t *resistance_buffer, volatile uint16_t *vrefanalog_buffer); // constructor with the led_driver, pins, adc_resistance and adc vref
    bool init();                                                                                                                                // init driver
    void handler();                                                                                                                             // handler needs to be called if new value should be generated
    void get_resistance(uint32_t channel_res[16]);                                                                                             // get the resistance
    void get_ignitable(uint8_t channel_ignitbale[16]);                                                                                         // get if a channel is ignitable
    void get_unused(uint8_t channel_unused[16]);                                                                                               // get i a channel is unused
    void setUsed(bool channel_needed[16]);                                                                                                      // set which channel is used
    void setBrightness(float brightness);                                                                                                       // set brightens                                                                                                                        // get if an error is received
    void resetCalculation();                                                                                                                    // reset the calculation
    void setBlink(bool blink);                                                                                                                  // should the led's which are unused be blinking
private:
    uint8_t _channel_pins[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       // channel pins
    uint32_t _channel_res[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};       // channel resistance accumulator
    uint32_t _channel_res_avg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   // channel resistance avg
    uint16_t _channel_counter_res = 0;                                                  // channel counter
    uint16_t _avg_counter_res = 0;                                                      // averaging counter
    uint8_t _channel_ignitbale[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // is the channel ignitable
    uint8_t _channel_pop_unused[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // is the channel populated
    bool _channel_needed[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};        // channel needed
    bool _led_blink_helper = 0;                                                         // helps with the blinking of the led's
    uint32_t _resistance = 0;                                                           // resistance value converted from adc reading
    volatile uint16_t *_adc_buffer;                                                     // resistance buffer adc count
    volatile uint16_t *_vref_buffer;                                                    // vref buffer adc count
    bool _led_driver_ok = 0;                                                            // is the led driver okay
    bool _led_blinking = 0;                                                             // should the led be blinking

    channel_led *_leds;
    Neotimer _resistance_timer;

    void convertADC();
    void calculate_resistance();
    void check_ignitable();
    void setChannelLEDsRes();
    void blinkLEDsRes();
    void copyArray8(uint8_t source[], uint8_t target[], int length);
    void copyArray32(uint32_t source[], uint32_t target[], int length);
    void copyArayBool(bool source[], bool target[], int length);
    void setAllOff();
};

#endif