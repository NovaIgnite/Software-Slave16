#include <resistance_control16.h>

/*
channel_ignitable table : 0 -> not ignitable / 1 -> ignitable / 2 -> not populated (open)
channel_unpopulated table: 0 -> not ignitable / 1 -> ignitable / 2 -> not populated (open) / 3 -> selected for program (no action needed/ignored)
channel_selected table: 0 -> not used in program / 1 -> used in program
*/

resistance_control::resistance_control(channel_led *led_channel, uint8_t pins[16], volatile uint16_t *resistance_buffer, volatile uint16_t *vrefanalog_buffer)
{
    copyArray8(pins, _channel_pins, 16); // copy the pins to the internal buffer
    _leds = led_channel;                 // set the led driver variable
    _adc_buffer = resistance_buffer;     // set the adc_buffer variable
    _vref_buffer = vrefanalog_buffer;    // set the vref_buffer variable
}
bool resistance_control::init()
{
    _resistance_timer = Neotimer(5); // start conversion timer with 5ms
    if (_leds->begin() == 0)         // setup driver
    {
        _leds->setLedBrightness(1.00); // set brightness to 100%
        _leds->clearAll();             // clear all led's
        _leds->setSleep(0);            // disable sleep
        setAllOff();                   // set all Channel MOSFETs to off
        _led_driver_ok = 1;            // flag driver okay
        return true;                   // return true (all good)
    }
    _led_driver_ok = 0; // flag driver fault
    return false;       // return false if no driver was found
}
void resistance_control::handler()
{
    calculate_resistance(); // calculate resistance
}
void resistance_control::get_resistance(uint32_t *channel_res[16])
{
    copyArray32(_channel_res_avg, *channel_res, 16); // copy calculated resistance to array
}
void resistance_control::get_ignitable(uint8_t *channel_ignitbale[16])
{
    copyArray8(_channel_ignitbale, *channel_ignitbale, 16); // copy which channel is ignitable to array
}
void resistance_control::get_unused(uint8_t *channel_unused[16])
{
    copyArray8(_channel_pop_unused, *channel_unused, 16); // copy which channel is unused to array
}
void resistance_control::setUsed(bool channel_needed[16])
{
    copyArayBool(channel_needed, _channel_needed, 16); // copy which channel need to be used to the internal buffer
}
void resistance_control::setBrightness(float brightness)
{
    _leds->setLedBrightness(brightness); // set brightness
}
void resistance_control::resetCalculation()
{
    _led_blink_helper = 0;
    _avg_counter_res = 0;
    _channel_counter_res = 0;
    // reset all the calculations to the reset value
}
void resistance_control::setBlink(bool blink)
{
    _led_blinking = blink; // change state if leds should be blinking when a unused channel is connected
}

void resistance_control::convertADC()
{
    _resistance = __HAL_ADC_CALC_DATA_TO_VOLTAGE(__HAL_ADC_CALC_VREFANALOG_VOLTAGE(*_vref_buffer, ADC_RESOLUTION_12B), *_adc_buffer, ADC_RESOLUTION_12B) / 2 * 100; // convert the adc counts with the vref to a resistance in miliohm
}
void resistance_control::calculate_resistance()
{
    if (_resistance_timer.repeat()) // if the next 5ms interval is over
    {
        if (_channel_counter_res < 16) // check if the limit of the channel count is reached
        {
            digitalWrite(_channel_pins[_channel_counter_res], HIGH); // set the corrsponding channel to high
            convertADC();                                            // convert to resistance
            if (_avg_counter_res < 6)                                // averaging counter 6 steps
            {
                if (_avg_counter_res > 0) // wait one loop to stabilize adc readings
                {
                    _channel_res_avg[_channel_counter_res] = _resistance + _channel_res_avg[_channel_counter_res]; // add to the averaging
                }
                _avg_counter_res++; // increment the averaging counter
            }
            else
            {
                // in the 7th loop
                _avg_counter_res = 0;                                   // reset the avg counter
                digitalWrite(_channel_pins[_channel_counter_res], LOW); // set the corresponding channel to low
                _channel_counter_res++;                                 // increment the selected channel
            }
        }
        else
        {
            _channel_counter_res = 0; // if all 16 channels are avg and saved reset the counter to 0
            for (int i = 0; i < 16; i++)
            {
                _channel_res[i] = _channel_res_avg[i] / 5; // for each of of the 16 channels calculate avg of the 5
                _channel_res_avg[i] = 0;                   // reset avg value
            }
            check_ignitable();   // check if the channel is ignitable
            setChannelLEDsRes(); // set the channel leds corresponding
            blinkLEDsRes();      // blink channels which are not used but still populated
        }
    }
}
void resistance_control::check_ignitable()
{
    for (int i = 0; i < 16; i++) // go through each channel
    {
        if (_channel_res[i] < 15000 && _channel_res[i] > 125) // if the resistance is bigger then 125mOhm and smaller then 15Ohm
        {
            _channel_ignitbale[i] = 1; // set the channel ignitable
        }
        else
        {
            _channel_ignitbale[i] = 0; // if not set them to not ignitable
        }
        if (_channel_res[i] > 100000) // if the resistance is bigger then 100Ohm
        {
            _channel_ignitbale[i] = 2; // set to not connected
        }
    }
}
void resistance_control::setChannelLEDsRes()
{
    if (_led_driver_ok == 1) // if the driver is okay
    {
        for (int i = 0; i < 16; i++) // go through each channel
        {
            if (_channel_needed[i] == 1) // if the channel is selected for the program
            {
                switch (_channel_ignitbale[i]) // check channel status
                {
                case 0:                             // if it is not ignitable
                    _leds->setLEDState(i, LED_RED); // set to RED
                    break;
                case 1:                               // if the channel is ignitable
                    _leds->setLEDState(i, LED_GREEN); // set to GREEN
                    break;
                case 2:                                // if the channel is still unpopulated
                    _leds->setLEDState(i, LED_YELLOW); // set to YELLOW
                    break;
                default:                            // anything else
                    _leds->setLEDState(i, LED_OFF); // turn LED OFF
                    break;
                }
                _channel_pop_unused[i] = 3; // set the flag in the unused array to used
            }
            else
            {
                _channel_pop_unused[i] = _channel_ignitbale[i]; // if the channel is not needed for ignition set the ignition state to the unused array
            }
        }
    }
}
void resistance_control::blinkLEDsRes()
{
    _led_blink_helper = !_led_blink_helper; // flip around the blink helper for blinking ~700mS

    for (int i = 0; i < 16; i++) // go through each channel
    {
        if (_led_blinking == 1) // if the channel should be blinking
        {
            if (_led_blink_helper == 0) // the blink helper says off
            {
                if (_channel_pop_unused[i] == 0 || _channel_pop_unused[i] == 1) // if the flag says ignitable or not set to OFF
                {
                    _leds->setLEDState(i, LED_OFF); // set the state of the channel to OFF
                }
            }
            if (_led_blink_helper == 1) // the blink helper says on
            {
                if (_channel_pop_unused[i] == 0) // channel is not ignitable
                {
                    _leds->setLEDState(i, LED_RED); // set LED to off
                }
                if (_channel_pop_unused[i] == 1) // channel is ignitable
                {
                    _leds->setLEDState(i, LED_GREEN); // set LED to on
                }
            }
            if (_channel_pop_unused[i] == 2) // if the channel is not populated and unused
            {
                _leds->setLEDState(i, LED_OFF); // turn off the LED
            }
        }
        else // if the channel should not be blinking
        {
            if (_channel_pop_unused[i] != 3) // if the channel is not selected for use
            {
                _leds->setLEDState(i, LED_OFF); // set the channel to off
            }
        }
    }
}

void resistance_control::copyArray8(uint8_t source[], uint8_t target[], int length)
{
    for (int i = 0; i < length; i++)
    {
        target[i] = source[i];
    }
    // copy a uint8 array
}
void resistance_control::copyArray32(uint32_t source[], uint32_t target[], int length)
{
    for (int i = 0; i < length; i++)
    {
        target[i] = source[i];
    }
    // copy a uint32 array
}
void resistance_control::copyArayBool(bool source[], bool target[], int length)
{
    for (int i = 0; i < length; i++)
    {
        target[i] = source[i];
    }
    // copy a bool array
}
void resistance_control::setAllOff()
{
    for (int i = 0; i < 16; i++)
    {
        digitalWrite(_channel_pins[i], LOW);
    }
    // set all of the MOSFETs to off
}