#ifndef _IS32FL3236A_h
#define _IS32FL3236A_h

#include <Arduino.h>
#include <Wire.h>

#define IS32FL3236A_OK    0x00
#define IS32FL3236A_ERROR 0xFF
#define IS32FL3236A_IMAX    0
#define IS32FL3236A_IMAX2   1
#define IS32FL3236A_IMAX3   2
#define IS32FL3236A_IMAX4   3

class IS32FL3236A{
    public:
        explicit  IS32FL3236A(uint8_t deviceAdress, uint8_t sdb_pin, TwoWire *wire = &Wire);
        bool begin();
        void blank(bool on = 0);
        void sleep(bool on);
        bool isConnected();
        void setLedPwm(uint8_t lednum, byte brightness);
        void setLedParam(uint8_t lednum, uint8_t current, bool enabled);
        void clear();
        void update();
        void setFrequency(bool high);
        void reset();
        uint8_t gamma64(uint8_t input);


    private:
        uint8_t _adress;
        uint8_t _sdbpin;
        uint8_t _data;
        int      _error;
        TwoWire* _wire;
        uint8_t writeReg(uint8_t reg, uint8_t value);
        uint8_t gamma64_table[64] = {0,1,2,3,4,5,6,7,8,10,12,14,16,18,20,22,24,26,29,32,35,38,41,44,47,50,53,57,61,65,69,73,77,81,85,89,94,99,104,109,114,119,124,129,134,140,146,152,158,164,170,176,182,188,195,202,209,216,223,230,237,244,251,255};
};

#endif
