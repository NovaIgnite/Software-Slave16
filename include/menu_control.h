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
    uint32_t resistance[8] = {0, 0, 0, 0, 0, 0, 0, 0};
} tres_screen;

typedef struct network_screen
{
    uint8_t device_type_A = 0;
    uint8_t device_type_B = 0;   

    uint8_t id_A = 0; 
    uint8_t id_B = 0; 

    uint8_t start_ch_A = 0; 
    uint8_t start_ch_B = 0; 

    bool en_A = 0;
    bool en_B = 0;
} tnetwork_screen;

class menu_control
{
public:
    menu_control(Adafruit_SSD1306 *display);
    bool init();
    void init_resistance_screen(tres_screen *screen);
    void init_arm_screen_manual();
    void init_arm_screen();
    void init_disarm_screen();
    void init_network_screen();
    void add_resistance(tres_screen *screen);
    void set_network_screen(tnetwork_screen *screen);
    void set_status(uint8_t status);
    void set_battery(uint8_t percentage);
    void set_charging(bool charge);
    void set_group(uint8_t letter);
    void set_device_number(uint8_t number);
    void set_arm_status(uint8_t percentage);
    void set_network_cursor(bool AB);

private:
    void clear_screen();
    void clear_dynamic_screen();
    void draw_status_bar();
    void draw_transmission_indicator(uint8_t number);
    void draw_battery_percentage(uint8_t percentage, bool charging);
    void draw_status(uint8_t status);
    void draw_device_number(uint8_t number);
    void draw_group_letter(uint8_t letter);
    void draw_arm_bar(uint8_t percentage);
    void draw_network_type(uint8_t type, bool AB);
    void draw_network_id(uint8_t id, bool AB);
    void draw_network_channel(uint8_t channel, bool AB);
    void draw_network_enabled_text(bool en, bool AB);

    String zeroPad(int number);
    String processOhm(uint32_t value);

    bool _charging = 0;
    uint8_t _percentage = 0;

    Adafruit_SSD1306 *_display;
};

#endif