#include <menu_control.h>
// constructor
menu_control::menu_control(Adafruit_SSD1306 *display)
{
    _display = display; // make variable neat
}
// public Functions
bool menu_control::init() // initialisation of display
{
    uint8_t error = _display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS); // begin and save error
    clear_screen();                                                      // clear entire screen
    draw_status_bar();                                                   // draw static part of the status bar
    return error;                                                        // send error back to system if any
}
void menu_control::init_resistance_screen(tres_screen *screen) // draw static part of resistance part
{
    clear_dynamic_screen(); // clear the dynamic screen part

    _display->drawLine(63, 20, 63, 64, WHITE); // draw the middle separator

    _display->setTextColor(WHITE); // set text color to white
    _display->setTextSize(1);      // set Text size to small

    uint8_t start_number_buffer = screen->start_number; // copy the start number to a buffer
    uint8_t resistance_counter = 0;                     // a counter for the array of resistances

    for (int i = 0; i < 2; i++) // for the two columns
    {
        for (int c = 0; c < 4; c++) // for the 4 rows
        {
            String buffer;                                                          // generate a String as a buffer
            _display->setCursor(2 + (i * 64), 22 + (c * 10));                       // set the cursor to a 2px margin and set it under the status bar
            buffer = "CH";                                                          // set CH label
            buffer = buffer + zeroPad(start_number_buffer);                         // pad the channel number to 3 digits and add
            buffer = buffer + ":";                                                  // add a :
            buffer = buffer + processOhm(screen->resistance[resistance_counter]);   // add the ohm value, it is pad'ed to 3 digits under 10Ohm with a decimal after without over 100 MAX is displayed
            _display->print(buffer);                                                // print to screen
            _display->drawChar(56 + (i * 64), 22 + (c * 10), 233, WHITE, BLACK, 1); // display OHM icon
            start_number_buffer++;                                                  // increment the start number fo next channel
            resistance_counter++;                                                   // increment the resistance counter so that the right resistance is the right value
        }
    }

    _display->display(); // send data to display
}
void menu_control::init_arm_screen_manual() // draw static part of the arm screen with a status bar
{
    clear_dynamic_screen();                    // clear the dynamic screen
    _display->setTextColor(WHITE);             // set text color to white
    _display->setTextSize(3);                  // set text size to very big
    _display->setCursor(10, 22);               // set cursor under status bar
    _display->print("ARMING");                 // print
    _display->setTextSize(1);                  // set text size to small
    _display->setCursor(4, 46);                // set cursor under big label
    _display->print("HOLD MAGNET OVER SW!");   // print
    _display->drawRect(12, 56, 102, 8, WHITE); // draw status bar
    _display->display();                       // send to display
    draw_arm_bar(0);                           // clear status bar
}
void menu_control::init_arm_screen() // draw static part of the remote arm screen
{
    clear_dynamic_screen();                  // clear the dynamic screen
    _display->setTextColor(WHITE);           // set text color to white
    _display->setTextSize(3);                // set text size to very big
    _display->setCursor(10, 28);             // set cursor under status bar and middle of y
    _display->print("ARMED!");               // print
    _display->setTextSize(1);                // set text size to small
    _display->setCursor(4, 51);              // set cursor under big label
    _display->print("USE MAGNET TO DISARM"); // print
    _display->display();                     // send to display
}
void menu_control::init_disarm_screen() // draw static part of the disarm screen
{
    clear_dynamic_screen();                  // clear the dynamic screen
    _display->setTextColor(WHITE);           // set text color to white
    _display->setTextSize(3);                // set text size to very big
    _display->setCursor(10, 28);             // set cursor under status bar and middle of y
    _display->print("DISARM");               // print
    _display->setTextSize(1);                // set text size to small
    _display->setCursor(4, 51);              // set cursor under big label
    _display->print("DISARMED SYSTEM SAFE"); // print
    _display->display();                     // send to display
}
void menu_control::init_network_screen() // draw static part of the network screen
{
    clear_dynamic_screen();                    // clear the dynamic screen
    _display->setTextColor(WHITE);             // set text color to white
    _display->setTextSize(0);                  // set text size to small
    _display->drawLine(63, 20, 63, 64, WHITE); // draw the middle separator

    _display->setCursor(8, 22);  // set cursor
    _display->print("TYPE:");    // draw type
    _display->setCursor(20, 32); // set cursor
    _display->print("ID:");      // draw ID
    _display->setCursor(2, 42);  // set cursor
    _display->print("SRTCH:");   // draw start channel

    _display->setCursor(72, 22); // set cursor
    _display->print("TYPE:");    // draw type
    _display->setCursor(84, 32); // set cursor
    _display->print("ID:");      // draw ID
    _display->setCursor(66, 42); // set cursor
    _display->print("SRTCH:");   // draw start channel

    _display->drawRect(2, 52, 60, 12, WHITE);  // draw left rectangle for enable button
    _display->drawRect(66, 52, 60, 12, WHITE); // draw right rectangle for enable button

    _display->display(); // display the static part

    draw_network_type(255, 0);
    draw_network_type(255, 1);
    draw_network_id(0, 0);
    draw_network_id(0, 1);
    draw_network_channel(0, 0);
    draw_network_channel(0, 1);
    draw_network_enabled_text(0, 0);
    draw_network_enabled_text(1, 1);
    set_network_cursor(0);
    // set default values
}
void menu_control::add_resistance(tres_screen *screen) // add dynamic parts of the resistance screen
{
    _display->setTextColor(WHITE); // text color WHITE
    _display->setTextSize(1);      // set the text size to small

    uint8_t resistance_counter = 0; // counter for resistance value

    for (int i = 0; i < 2; i++) // for the two columns
    {
        for (int c = 0; c < 4; c++) // for the 4 rows
        {
            _display->fillRect(38 + (i * 64), 22 + (c * 10), 18, 8, BLACK); // clear old value
            String buffer;                                                  // generate a String as a buffer
            _display->setCursor(38 + (i * 64), 22 + (c * 10));              // set the cursor to a 2px margin and set it under the status bar and behind the label CHxxx:
            buffer = processOhm(screen->resistance[resistance_counter]);    // add the ohm value, it is pad'ed to 3 digits under 10Ohm with a decimal after without over 100 MAX is displayed
            _display->print(buffer);                                        // print to screen
            resistance_counter++;                                           // increment the resistance counter so that the right resistance is the right value
        }
    }

    _display->display(); // send data to display
}
void menu_control::add_network_screen(tnetwork_screen *screen) // updates newtork screen from the provided struct
{
    draw_network_type(screen->device_type_A, 0); // draws type of attached device A
    draw_network_type(screen->device_type_B, 0); // draws type of attached device B
    draw_network_id(screen->id_A, 0);            // draws device ID of attached device A
    draw_network_id(screen->id_B, 1);            // draws device ID of attached device B
    draw_network_channel(screen->start_ch_A, 0); // draws start channel of attached device A
    draw_network_channel(screen->start_ch_B, 1); // draws start channel of attached device
    draw_network_enabled_text(screen->en_A, 0);  // draws if the device A is enabled
    draw_network_enabled_text(screen->en_B, 0);  // draws if the device Bis enabled
}
void menu_control::set_status(uint8_t status) // setter function for the status
{
    draw_status(status); // status setter
}
void menu_control::set_battery(uint8_t percentage) // setter function for battery percentage
{
    _percentage = percentage;                        // save battery percentage
    draw_battery_percentage(_percentage, _charging); // set battery value with saved values
}
void menu_control::set_charging(bool charge) // setter function for setting the charging status
{
    _charging = charge;                              // save charging value
    draw_battery_percentage(_percentage, _charging); // set battery value with saved values
}
void menu_control::set_group(uint8_t letter) // setter function for the group letter
{
    draw_group_letter(letter); // group letter setter
}
void menu_control::set_device_number(uint8_t number) // setter function for the device number
{
    draw_device_number(number); // device number setter
}
void menu_control::set_arm_status(uint8_t percentage) // setter function for the arm bar of the manual arm screen
{
    draw_arm_bar(percentage); // arm status bar setter
}
void menu_control::set_network_cursor(bool AB) // draw the cursor for selecting the A or B device and enable it
{
    _display->setTextColor(WHITE); // text color white
    _display->setTextSize(1);      // small text

    if (AB == 0) // check AB so left and right
    {
        _display->fillRect(6, 54, 12, 8, BLACK); // draw over old arrow
        _display->setCursor(6, 54);              // set cursor
        _display->print("->");                   // draw arrow
    }
    if (AB == 1) // check AB so left and right
    {
        _display->fillRect(70, 54, 12, 8, BLACK); // draw over old arrow
        _display->setCursor(70, 54);              // set cursor
        _display->print("->");                    // draw arrow
    }

    _display->display(); // print to screen
}

// private Functions
void menu_control::clear_screen() // clears the entire screen
{
    _display->clearDisplay(); // clear the entire screen
    _display->display();      // send to display
}
void menu_control::clear_dynamic_screen() // only clears the part under the status bar
{
    _display->fillRect(0, 21, 128, 43, BLACK); // clear only part under status bar
    _display->display();                       // send to display
}
void menu_control::draw_transmission_indicator(uint8_t number) // draws the tranmission strength indicator, and also the wired symbol in the status bar
{
    if (number <= 5 && number >= 0) // check if number is in range
    {
        _display->fillRect(77, 3, 16, 16, BLACK); // clear old image
        switch (number)
        {
        case 0:
            _display->drawBitmap(77, 3, transmission_zero, 16, 16, WHITE); // no signal
            break;
        case 1:
            _display->drawBitmap(77, 3, transmission_low, 16, 16, WHITE); // low signal
            break;
        case 2:
            _display->drawBitmap(77, 3, transmission_mid, 16, 16, WHITE); // middle signal
            break;
        case 3:
            _display->drawBitmap(77, 3, transmission_high, 16, 16, WHITE); // good signal
            break;
        case 4:
            _display->drawBitmap(77, 3, transmission_full, 16, 16, WHITE); // very good signal
            break;
        case 5:
            _display->drawBitmap(77, 3, transmission_placeholder, 16, 16, WHITE); // placeholder
            break;
        default:
            break;
        }
        _display->display(); // send to display
    }
    else
    {
        return;
    }
}
void menu_control::draw_battery_percentage(uint8_t percentage, bool charging) // draws the battery percentage in the status bar
{
    _display->setTextSize(1); // small text
    if (charging == 0)        // if no charging is selected
    {
        _display->setTextColor(WHITE);            // set color to White
        _display->fillRect(96, 4, 28, 13, BLACK); // draw over old stuff
    }
    else
    {
        _display->setTextColor(BLACK);            // set color to black
        _display->fillRect(96, 4, 28, 13, WHITE); // draw over old stuff
    }

    if (percentage <= 100 && percentage >= 0) // check if in range
    {
        String buffer;
        buffer = buffer + zeroPad(percentage); // pad String with zero
        buffer = buffer + "%";                 // add the % sign
        buffer = buffer + "\0";                // zero terminate
        _display->setCursor(99, 7);            // set cursor
        _display->print(buffer);               // print buffer
    }
    else
    {
        _display->setCursor(99, 7); // set cursor
        _display->print("---%");    // print placeholder
    }
    _display->display(); // send to display
}
void menu_control::draw_status(uint8_t status) // draws the status message in the status bar
{
    _display->setTextColor(WHITE);           // text color white
    _display->setTextSize(2);                // text size bigger
    _display->fillRect(2, 3, 48, 16, BLACK); // draw over old stuff
    _display->setCursor(2, 3);               // set cursor
    switch (status)                          // check which status
    {
    case 0:
        _display->print("SAFE");
        break;
    case 1:
        _display->print("PARM");
        break;
    case 2:
        _display->print("ARM");
        break;
    case 3:
        _display->print("FIRE");
        break;
    case 4:
        _display->print("GRPD");
        break;
    case 5:
        _display->print("LOAD");
        break;
    case 6:
        _display->print("WLSC");
        break;
    case 7:
        _display->print("STBY");
        break;
    case 8:
        _display->print("SYNC");
        break;
    case 9:
        _display->print("SRCH");
        break;
    case 245:
        _display->print("E001");
        break;
    case 246:
        _display->print("E002");
        break;
    case 247:
        _display->print("E003");
        break;
    case 248:
        _display->print("E004");
        break;
    case 249:
        _display->print("E005");
        break;
    case 250:
        _display->print("E006");
        break;
    case 251:
        _display->print("E007");
        break;
    case 252:
        _display->print("E008");
        break;
    case 253:
        _display->print("E009");
        break;
    case 254:
        _display->print("E010");
        break;
    case 255:
        _display->print("----");
        break;

    default:
        break;
    }
    _display->display(); // send to display
}
void menu_control::draw_device_number(uint8_t number) // draws the device number in the status bar
{
    _display->setTextColor(WHITE);           // text color white
    _display->setTextSize(1);                // small text
    _display->fillRect(55, 2, 18, 8, BLACK); // draw over old stuff
    _display->setCursor(55, 2);              // set cursor
    if (number > 0)
    {
        _display->print(zeroPad(number)); // pad number with zeros and print
    }
    else
    {
        _display->print("---"); // draw placeholder
    }
    _display->display(); // send to display
}
void menu_control::draw_group_letter(uint8_t letter) // draws the group letter in the status bar
{
    _display->setTextColor(WHITE);           // text color white
    _display->setTextSize(1);                // small text
    _display->fillRect(61, 12, 6, 8, BLACK); // write over old letter
    _display->setCursor(61, 12);             // set the cursor to the right place
    if (letter <= 26 && letter >= 0)         // check if number is in bounds
    {
        switch (letter) // draw the right letter alphabet + spacer(-)
        {
        case 0:
            _display->print("A");
            break;
        case 1:
            _display->print("B");
            break;
        case 2:
            _display->print("C");
            break;
        case 3:
            _display->print("D");
            break;
        case 4:
            _display->print("E");
            break;
        case 5:
            _display->print("F");
            break;
        case 6:
            _display->print("G");
            break;
        case 7:
            _display->print("H");
            break;
        case 8:
            _display->print("I");
            break;
        case 9:
            _display->print("J");
            break;
        case 10:
            _display->print("K");
            break;
        case 11:
            _display->print("L");
            break;
        case 12:
            _display->print("M");
            break;
        case 13:
            _display->print("N");
            break;
        case 14:
            _display->print("O");
            break;
        case 15:
            _display->print("P");
            break;
        case 16:
            _display->print("Q");
            break;
        case 17:
            _display->print("R");
            break;
        case 18:
            _display->print("S");
            break;
        case 19:
            _display->print("T");
            break;
        case 20:
            _display->print("U");
            break;
        case 21:
            _display->print("V");
            break;
        case 22:
            _display->print("W");
            break;
        case 23:
            _display->print("X");
            break;
        case 24:
            _display->print("Y");
            break;
        case 25:
            _display->print("Z");
            break;
        case 26:
            _display->print("-");
            break;

        default:
            break;
        }
        _display->display(); // send to display
    }
    else
    {
        return;
    }
}
void menu_control::draw_arm_bar(uint8_t percentage) // draws the arm bar on the manual arm screen
{
    if (percentage <= 100 && percentage >= 0) // check range of numbers
    {
        _display->fillRect(13, 57, 100, 6, BLACK);        // draw over old bar
        _display->fillRect(13, 57, percentage, 6, WHITE); // draw a filled rect as the status bar from 0 to 100%
        _display->display();                              // send to display
    }
}
void menu_control::draw_network_type(uint8_t type, bool AB) // prints the type of detected slat box to the field
{
    _display->setTextColor(WHITE); // text color white
    _display->setTextSize(1);      // small text
    if (AB == 0)
    {
        _display->fillRect(38, 22, 24, 8, BLACK); // write over old letter
        _display->setCursor(38, 22);              // set the cursor to the right place
    }
    if (AB == 1)
    {
        _display->fillRect(102, 22, 24, 8, BLACK); // write over old letter
        _display->setCursor(102, 22);              // set the cursor to the right place
    }

    switch (type) // switch for all the types
    {
    case 0:
        _display->print("8CH"); // print 8 channel
        break;
    case 1:
        _display->print("16CH"); // print 16 channel
        break;
    case 3:
        _display->print("24CH"); // print 24 channel
        break;
    case 4:
        _display->print("32CH"); // print 32 channel
        break;
    case 5:
        _display->print("48CH"); // print 48 channel
        break;
    case 6:
        _display->print("64CH"); // print 64 channel
        break;
    default:
        _display->print("----"); // print placeholder
        break;
    }
    _display->display(); // print to screen
}
void menu_control::draw_network_id(uint8_t id, bool AB) // prints the network id to the screen
{
    _display->setTextColor(WHITE); // text color white
    _display->setTextSize(1);      // small text
    if (AB == 0)
    {
        _display->fillRect(38, 32, 18, 8, BLACK); // write over old letter
        _display->setCursor(38, 32);              // set the cursor to the right place
    }
    if (AB == 1)
    {
        _display->fillRect(102, 32, 18, 8, BLACK); // write over old letter
        _display->setCursor(102, 32);              // set the cursor to the right place
    }
    if (id > 0) // check if id is valid
    {
        _display->print(zeroPad(id)); // print id of network device padded to 3 digits
    }
    else
    {
        _display->print("---"); // print --- as placeholder
    }
    _display->display(); // print to screen
}
void menu_control::draw_network_channel(uint8_t channel, bool AB) // prints the start channel to the screen
{
    _display->setTextColor(WHITE); // text color white
    _display->setTextSize(1);      // small text
    if (AB == 0)
    {
        _display->fillRect(38, 42, 18, 8, BLACK); // write over old letter
        _display->setCursor(38, 42);              // set the cursor to the right place
    }
    if (AB == 1)
    {
        _display->fillRect(102, 42, 18, 8, BLACK); // write over old letter
        _display->setCursor(102, 42);              // set the cursor to the right place
    }
    if (channel > 0) // check if start channel is valid
    {
        _display->print(zeroPad(channel)); // print start channel of network device padded to 3 digits
    }
    else
    {
        _display->print("---"); // print --- as placeholder
    }
    _display->display(); // print to screen
}
void menu_control::draw_network_enabled_text(bool en, bool AB) // draw the text if the power to the network is enabled or not
{
    _display->setTextColor(WHITE); // text color white
    _display->setTextSize(1);      // small text
    if (AB == 0)
    {
        _display->fillRect(22, 54, 36, 8, BLACK); // write over old letter
        _display->setCursor(22, 54);              // set the cursor to the right place
    }
    if (AB == 1)
    {
        _display->fillRect(86, 54, 36, 8, BLACK); // write over old letter
        _display->setCursor(86, 54);              // set the cursor to the right place
    }

    if (en == 0 && AB == 0)
    {
        _display->print("ENBL A");
    }
    if (en == 1 && AB == 0)
    {
        _display->print("A OPRT");
    }

    if (en == 0 && AB == 1)
    {
        _display->print("ENBL B");
    }
    if (en == 1 && AB == 1)
    {
        _display->print("B OPRT");
    }

    _display->display();
}
void menu_control::draw_status_bar() // draw status bar of static screen
{
    _display->drawLine(0, 20, 128, 20, WHITE);  // draw limiter line
    _display->drawRect(95, 3, 30, 15, WHITE);   // draw battery rect
    _display->drawLine(125, 7, 125, 13, WHITE); // draw battery +pole line 1
    _display->drawLine(126, 7, 126, 13, WHITE); // draw battery +pole line 2
    _display->drawLine(51, 2, 51, 18, WHITE);   // draw first separator
    _display->drawLine(52, 2, 52, 18, WHITE);   // draw second separator
    _display->drawLine(52, 10, 73, 10, WHITE);  // draw horizontal spacer
    _display->drawLine(74, 2, 74, 18, WHITE);   // draws first second separator
    _display->drawLine(75, 2, 75, 18, WHITE);   // draws second second separator

    draw_device_number(0);           // draws --- as a placeholder in the device number spot
    draw_battery_percentage(255, 0); // draws --- as a placeholder in the battery spot
    draw_status(255);                // draws ---- as a placeholder in the status field
    draw_transmission_indicator(5);  // draws --- as a placeholder in the transmission strength indicator
    draw_group_letter(26);           // draws - as a placeholder in the group letter spot
}

String menu_control::zeroPad(int number) // this function pads values from 0-999 with 0s
{
    // Ensure the number is within the 0-999 range
    if (number < 0)
    {
        number = 0;
    }
    else if (number > 999)
    {
        number = 999;
    }

    // Use sprintf to format the number with leading zeros
    char buffer[4]; // 3 digits + null terminator
    sprintf(buffer, "%03d", number);

    return String(buffer);
}
String menu_control::processOhm(uint32_t value) // this function converts the calculated resistance values to drawable Strings
{
    // Divide the value by 1000.0 to get the float result
    float result = value / 1000.0;

    // Check if the result is greater than or equal to 100
    if (result >= 100.0)
    {
        return "MAX";
    }

    // Prepare a buffer to hold the formatted string
    char buffer[8]; // Enough space for "99.9" format plus null terminator

    // Format the result based on its value
    if (result >= 10.0)
    {
        // Print without decimal point if result is over 10.0
        sprintf(buffer, "%03d", (int)result);
    }
    else
    {
        // Print with one decimal point if result is less than 10.0
        sprintf(buffer, "%03.1f", result);
    }

    // Return the formatted string
    return String(buffer);
}