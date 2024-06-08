#include <menu_control.h>

menu_control::menu_control(Adafruit_SSD1306 *display)
{
    _display = display; // make variable neat
}
bool menu_control::init()
{
    uint8_t error = _display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS); // beginn and save error
    clear_screen();                                                      // clear entire screen
    draw_status_bar();                                                   // draw static part of the status bar
    return error;                                                        // send error back to system if any
}
void menu_control::init_resistance_screen(res_screen *screen)
{
    clear_dynamic_screen();

    _display->drawLine(63, 20, 63, 64, WHITE);

    _display->setTextColor(WHITE);
    _display->setTextSize(1);

    uint8_t start_number_buffer = screen->start_number;
    uint8_t resistance_counter = 0;

    for (int i = 0; i < 2; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            String buffer;
            _display->setCursor(2 + (i * 64), 22 + (c * 10));
            buffer = "CH";
            buffer = buffer + zeroPad(start_number_buffer);
            buffer = buffer + ":";
            buffer = buffer + processOhm(screen->resistance[resistance_counter]);
            _display->print(buffer);
            _display->drawChar(56 + (i * 64), 22 + (c * 10), 233, WHITE, BLACK, 1);
            start_number_buffer++;
            resistance_counter++;
        }
    }

    _display->display();
}
void menu_control::add_resistance(res_screen *screen)
{
    _display->setTextColor(WHITE);
    _display->setTextSize(1);

    uint8_t resistance_counter = 0;

    for (int i = 0; i < 2; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            _display->fillRect(38 + (i * 64), 22 + (c * 10),18,8,BLACK);
            String buffer;
            _display->setCursor(38 + (i * 64), 22 + (c * 10));
            buffer = processOhm(screen->resistance[resistance_counter]);
            _display->print(buffer);
            resistance_counter++;
        }
    }

    _display->display();   
}
void menu_control::draw_status_bar()
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
void menu_control::clear_screen()
{
    _display->clearDisplay(); // clear the entire screen
    _display->display();      // send to display
}
void menu_control::clear_dynamic_screen()
{
    _display->fillRect(0, 21, 128, 43, BLACK); // clear only part under status bar
    _display->display();                       // send to display
}
void menu_control::draw_transmission_indicator(uint8_t number)
{
    if (number <= 5 && number >= 0)
    {
        _display->fillRect(77, 3, 16, 16, BLACK);
        switch (number)
        {
        case 0:
            _display->drawBitmap(77, 3, transmission_zero, 16, 16, WHITE);
            break;
        case 1:
            _display->drawBitmap(77, 3, transmission_low, 16, 16, WHITE);
            break;
        case 2:
            _display->drawBitmap(77, 3, transmission_mid, 16, 16, WHITE);
            break;
        case 3:
            _display->drawBitmap(77, 3, transmission_high, 16, 16, WHITE);
            break;
        case 4:
            _display->drawBitmap(77, 3, transmission_full, 16, 16, WHITE);
            break;
        case 5:
            _display->drawBitmap(77, 3, transmission_placeholder, 16, 16, WHITE);
            break;
        default:
            break;
        }
        _display->display();
    }
    else
    {
        return;
    }
}
void menu_control::draw_battery_percentage(uint8_t percentage, bool charging)
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
void menu_control::draw_status(uint8_t status)
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
void menu_control::draw_device_number(uint8_t number)
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
void menu_control::draw_group_letter(uint8_t letter)
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
String menu_control::zeroPad(int number)
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
String menu_control::processOhm(uint32_t value)
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