#include "IS32FL3236A.h"

IS32FL3236A::IS32FL3236A(uint8_t deviceAdress, uint8_t sdb_pin, TwoWire *wire)
{
    _adress = deviceAdress;
    _wire = wire;
    _sdbpin = sdb_pin;
}

bool IS32FL3236A::begin()
{
    _wire->begin();
    pinMode(_sdbpin, OUTPUT);
    digitalWrite(_sdbpin, LOW);
    if (!isConnected())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool IS32FL3236A::isConnected()
{
    _wire->beginTransmission(_adress);
    _error = _wire->endTransmission();
    return (_error == 0);
}
void IS32FL3236A::blank(bool on)
{
    if (on == 0)
    {
        writeReg(0x4A, 0x01);
    }
    if (on == 1)
    {
        writeReg(0x4A, 0x00);
    }
}
void IS32FL3236A::sleep(bool on)
{
    if (on == 0)
    {
        writeReg(0x00, 0x01);
        digitalWrite(_sdbpin, HIGH);
    }
    if (on == 1)
    {
        writeReg(0x00, 0x00);
        digitalWrite(_sdbpin, LOW);
    }
}
void IS32FL3236A::setLedPwm(uint8_t lednum, byte brightness)
{
    uint8_t hexlednum = lednum + 0x01;
    writeReg(hexlednum, brightness);
}
void IS32FL3236A::setLedParam(uint8_t lednum, uint8_t current, bool enabled)
{
    uint8_t hexlednum = lednum + 0x26;
    uint8_t hexparam;
    if (current > 3)
    {
        current == 3;
    }
    if (current < 0)
    {
        current == 0;
    }
    bitWrite(hexparam, 7, 0);
    bitWrite(hexparam, 6, 0);
    bitWrite(hexparam, 5, 0);
    bitWrite(hexparam, 4, 0);
    bitWrite(hexparam, 3, 0);
    bitWrite(hexparam, 2, bitRead(current, 1));
    bitWrite(hexparam, 1, bitRead(current, 0));
    bitWrite(hexparam, 0, bitRead(enabled, 0));

    writeReg(hexlednum, hexparam);
}
void IS32FL3236A::clear()
{
    for (int i = 0x01; i <= 0x24; i++)
    {
        writeReg(i, 0x00);
    }
}
void IS32FL3236A::update()
{
    writeReg(0x25, 0x00);
}
void IS32FL3236A::setFrequency(bool high)
{
    writeReg(0x48, high);
}
void IS32FL3236A::reset()
{
    writeReg(0x4F, 0x00);
}
uint8_t IS32FL3236A::gamma64(uint8_t input)
{
    uint8_t output = gamma64_table[input];
    return output;
}

uint8_t IS32FL3236A::writeReg(uint8_t reg, uint8_t value)
{
    _wire->beginTransmission(_adress);
    _wire->write(reg);
    _wire->write(value);
    _error = _wire->endTransmission();
    if (_error == 0)
    {
        _error = IS32FL3236A_OK;
    }
    else
    {
        _error = IS32FL3236A_ERROR;
    }
    return _error;
}
