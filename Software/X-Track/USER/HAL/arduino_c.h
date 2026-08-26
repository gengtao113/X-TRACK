#ifndef __ARDUINO_C_H
#define __ARDUINO_C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     Serial_Begin(uint32_t baud);
void     Serial_Print(const char* s);
void     Serial_Println(const char* s);
void     Serial_Printf(const char* fmt, ...);
void     Serial_PrintHex(uint8_t v);
void     Serial_WriteBuf(const unsigned char* buf, unsigned len);
int      Serial_Available(void);
int      Serial_Read(void);
void     Serial_Write(uint8_t c);

void     Serial2_Begin(uint32_t baud);
int      Serial2_Available(void);
int      Serial2_Read(void);
void     Serial2_Write(uint8_t c);

void     Arduino_Tone(int pin, uint32_t freq, int32_t duration);

int      Wire_Begin(void);
void     Wire_BeginTransmission(uint8_t addr);
int      Wire_EndTransmission(void);

#ifdef __cplusplus
}
#endif

#endif
