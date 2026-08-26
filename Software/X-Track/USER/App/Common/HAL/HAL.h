#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "HAL_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*CommitFunc_t)(void* info, void* userData);
typedef void (*Display_CallbackFunc_t)(void);
typedef void (*SD_CallbackFunction_t)(bool insert);
typedef void (*Power_CallbackFunction_t)(void);

void HAL_Init(void);
void HAL_Update(void);

void Backlight_Init(void);
uint16_t Backlight_GetValue(void);
void Backlight_SetValue(int16_t val);
void Backlight_SetGradual(uint16_t target, uint16_t time);
void Backlight_ForceLit(bool en);

void Display_Init(void);
void Display_DumpCrashInfo(const char* info);
void Display_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void Display_SendPixels(const uint16_t* pixels, uint32_t len);
void Display_SetSendFinishCallback(Display_CallbackFunc_t func);

void FaultHandle_Init(void);

int I2C_Scan(void);

bool IMU_Init(void);
void IMU_SetCommitCallback(CommitFunc_t func, void* userData);
void IMU_Update(void);

bool MAG_Init(void);
void MAG_SetCommitCallback(CommitFunc_t func, void* userData);
void MAG_Update(void);

bool SD_Init(void);
void SD_Update(void);
bool SD_GetReady(void);
float SD_GetCardSizeMB(void);
const char* SD_GetTypeName(void);
void SD_SetEventCallback(SD_CallbackFunction_t callback);

void Power_Init(void);
void Power_HandleTimeUpdate(void);
void Power_SetAutoLowPowerTimeout(uint16_t sec);
uint16_t Power_GetAutoLowPowerTimeout(void);
void Power_SetAutoLowPowerEnable(bool en);
void Power_Shutdown(void);
void Power_Update(void);
void Power_EventMonitor(void);
void Power_GetInfo(Power_Info_t* info);
void Power_SetEventCallback(Power_CallbackFunction_t callback);

void Clock_Init(void);
void Clock_GetInfo(Clock_Info_t* info);
void Clock_SetInfo(const Clock_Info_t* info);
const char* Clock_GetWeekString(uint8_t week);

void GPS_Init(void);
void GPS_Update(void);
bool GPS_GetInfo(GPS_Info_t* info);
bool GPS_LocationIsValid(void);
double GPS_GetDistanceOffset(GPS_Info_t* info, double preLong, double preLat);

void Buzz_init(void);
void Buzz_SetEnable(bool en);
void Buzz_Tone(uint32_t freq, int32_t duration);

void Encoder_Init(void);
void Encoder_Update(void);
int32_t Encoder_GetDiff(void);
bool Encoder_GetIsPush(void);
void Encoder_SetEnable(bool en);

void Audio_Init(void);
void Audio_Update(void);
bool Audio_PlayMusic(const char* name);

void Memory_DumpInfo(void);

#ifdef __cplusplus
}

namespace HAL
{

typedef ::CommitFunc_t CommitFunc_t;
typedef ::Display_CallbackFunc_t Display_CallbackFunc_t;
typedef ::SD_CallbackFunction_t SD_CallbackFunction_t;
typedef ::Power_CallbackFunction_t Power_CallbackFunction_t;

inline void HAL_Init() { ::HAL_Init(); }
inline void HAL_Update() { ::HAL_Update(); }

inline void Backlight_Init() { ::Backlight_Init(); }
inline uint16_t Backlight_GetValue() { return ::Backlight_GetValue(); }
inline void Backlight_SetValue(int16_t val) { ::Backlight_SetValue(val); }
inline void Backlight_SetGradual(uint16_t target, uint16_t time = 500) { ::Backlight_SetGradual(target, time); }
inline void Backlight_ForceLit(bool en) { ::Backlight_ForceLit(en); }

inline void Display_Init() { ::Display_Init(); }
inline void Display_DumpCrashInfo(const char* info) { ::Display_DumpCrashInfo(info); }
inline void Display_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1) { ::Display_SetAddrWindow(x0, y0, x1, y1); }
inline void Display_SendPixels(const uint16_t* pixels, uint32_t len) { ::Display_SendPixels(pixels, len); }
inline void Display_SetSendFinishCallback(Display_CallbackFunc_t func) { ::Display_SetSendFinishCallback(func); }

inline void FaultHandle_Init() { ::FaultHandle_Init(); }

inline int I2C_Scan() { return ::I2C_Scan(); }

inline bool IMU_Init() { return ::IMU_Init(); }
inline void IMU_SetCommitCallback(CommitFunc_t func, void* userData) { ::IMU_SetCommitCallback(func, userData); }
inline void IMU_Update() { ::IMU_Update(); }

inline bool MAG_Init() { return ::MAG_Init(); }
inline void MAG_SetCommitCallback(CommitFunc_t func, void* userData) { ::MAG_SetCommitCallback(func, userData); }
inline void MAG_Update() { ::MAG_Update(); }

inline bool SD_Init() { return ::SD_Init(); }
inline void SD_Update() { ::SD_Update(); }
inline bool SD_GetReady() { return ::SD_GetReady(); }
inline float SD_GetCardSizeMB() { return ::SD_GetCardSizeMB(); }
inline const char* SD_GetTypeName() { return ::SD_GetTypeName(); }
inline void SD_SetEventCallback(SD_CallbackFunction_t callback) { ::SD_SetEventCallback(callback); }

inline void Power_Init() { ::Power_Init(); }
inline void Power_HandleTimeUpdate() { ::Power_HandleTimeUpdate(); }
inline void Power_SetAutoLowPowerTimeout(uint16_t sec) { ::Power_SetAutoLowPowerTimeout(sec); }
inline uint16_t Power_GetAutoLowPowerTimeout() { return ::Power_GetAutoLowPowerTimeout(); }
inline void Power_SetAutoLowPowerEnable(bool en) { ::Power_SetAutoLowPowerEnable(en); }
inline void Power_Shutdown() { ::Power_Shutdown(); }
inline void Power_Update() { ::Power_Update(); }
inline void Power_EventMonitor() { ::Power_EventMonitor(); }
inline void Power_GetInfo(Power_Info_t* info) { ::Power_GetInfo(info); }
inline void Power_SetEventCallback(Power_CallbackFunction_t callback) { ::Power_SetEventCallback(callback); }

inline void Clock_Init() { ::Clock_Init(); }
inline void Clock_GetInfo(Clock_Info_t* info) { ::Clock_GetInfo(info); }
inline void Clock_SetInfo(const Clock_Info_t* info) { ::Clock_SetInfo(info); }
inline const char* Clock_GetWeekString(uint8_t week) { return ::Clock_GetWeekString(week); }

inline void GPS_Init() { ::GPS_Init(); }
inline void GPS_Update() { ::GPS_Update(); }
inline bool GPS_GetInfo(GPS_Info_t* info) { return ::GPS_GetInfo(info); }
inline bool GPS_LocationIsValid() { return ::GPS_LocationIsValid(); }
inline double GPS_GetDistanceOffset(GPS_Info_t* info, double preLong, double preLat) { return ::GPS_GetDistanceOffset(info, preLong, preLat); }

inline void Buzz_init() { ::Buzz_init(); }
inline void Buzz_SetEnable(bool en) { ::Buzz_SetEnable(en); }
inline void Buzz_Tone(uint32_t freq, int32_t duration = -1) { ::Buzz_Tone(freq, duration); }

inline void Encoder_Init() { ::Encoder_Init(); }
inline void Encoder_Update() { ::Encoder_Update(); }
inline int32_t Encoder_GetDiff() { return ::Encoder_GetDiff(); }
inline bool Encoder_GetIsPush() { return ::Encoder_GetIsPush(); }
inline void Encoder_SetEnable(bool en) { ::Encoder_SetEnable(en); }

inline void Audio_Init() { ::Audio_Init(); }
inline void Audio_Update() { ::Audio_Update(); }
inline bool Audio_PlayMusic(const char* name) { return ::Audio_PlayMusic(name); }

inline void Memory_DumpInfo() { ::Memory_DumpInfo(); }

}

#endif

#endif
