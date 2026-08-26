#include "arduino_c.h"
#include "Arduino.h"
#include "Wire.h"
#include <stdio.h>
#include <stdarg.h>

void Serial_Begin(uint32_t baud)
{
    Serial.begin(baud);
}

void Serial_Print(const char* s)
{
    Serial.print(s);
}

void Serial_Println(const char* s)
{
    Serial.println(s);
}

void Serial_Printf(const char* fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print(buf);
}

void Serial_PrintHex(uint8_t v)
{
    Serial.print(v, HEX);
}

void Serial_WriteBuf(const unsigned char* buf, unsigned len)
{
    Serial.write(buf, len);
}

int Serial_Available(void)
{
    return Serial.available();
}

int Serial_Read(void)
{
    return Serial.read();
}

void Serial_Write(uint8_t c)
{
    Serial.write(c);
}

void Serial2_Begin(uint32_t baud)
{
    Serial2.begin(baud);
}

int Serial2_Available(void)
{
    return Serial2.available();
}

int Serial2_Read(void)
{
    return Serial2.read();
}

void Serial2_Write(uint8_t c)
{
    Serial2.write(c);
}

void Arduino_Tone(int pin, uint32_t freq, int32_t duration)
{
    if (duration >= 0)
    {
        tone(pin, freq, (unsigned long)duration);
    }
    else
    {
        tone(pin, freq);
    }
}

int Wire_Begin(void)
{
    return Wire.begin() ? 1 : 0;
}

void Wire_BeginTransmission(uint8_t addr)
{
    Wire.beginTransmission(addr);
}

int Wire_EndTransmission(void)
{
    return Wire.endTransmission();
}
