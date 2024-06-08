#ifndef _menu_control_h
#define _menu_control_h

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pins.h>
#include <icons.h>

typedef struct res_screen
{
    uint8_t start_number = 0;
    uint32_t resistance[8] = {0,0,0,0,0,0,0,0};
} tres_screen;



class menu_control
{
public:
    menu_control(Adafruit_SSD1306 *display);
    bool init();
    void init_resistance_screen(res_screen *screen);
    void handler();
private:
    void clear_screen();
    void clear_dynamic_screen();
    void draw_status_bar();
    void draw_transmission_indicator(uint8_t number);
    void draw_battery_percentage(uint8_t percentage,bool charging);
    void draw_status(uint8_t status);
    void draw_device_number(uint8_t number);
    void draw_group_letter(uint8_t letter);


    String zeroPad(int number);
    String processOhm(uint32_t value);

    Adafruit_SSD1306 *_display;
};

#endif