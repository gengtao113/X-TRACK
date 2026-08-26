#ifndef __HAL_DEV_H
#define __HAL_DEV_H

#include <stdint.h>
#include <stdbool.h>
#include "App/Common/HAL/HAL_Def.h"
#include "App/Utils/TonePlayer/TonePlayer_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEV_IMU_INFO IMU_Info_t
#define DEV_MAG_INFO MAG_Info_t
#define DEV_GPS_INFO GPS_Info_t

enum
{
    DEV_BTN_EVENT_NONE = 0,
    DEV_BTN_EVENT_PRESSED,
    DEV_BTN_EVENT_PRESSING,
    DEV_BTN_EVENT_LONG_PRESSED,
    DEV_BTN_EVENT_LONG_PRESSED_REPEAT,
    DEV_BTN_EVENT_LONG_PRESSED_RELEASED,
    DEV_BTN_EVENT_RELEASED
};

bool        DevIMU_Init(void);
void        DevIMU_GetMotion(DEV_IMU_INFO* info);

bool        DevMAG_Init(void);
void        DevMAG_Read(DEV_MAG_INFO* info);

const char* DevGPS_LibraryVersion(void);
void        DevGPS_Encode(char c);
bool        DevGPS_FillInfo(DEV_GPS_INFO* info);
bool        DevGPS_LocationValid(void);
double      DevGPS_Distance(double lat1, double lon1, double lat2, double lon2);

void*       DevBtn_Create(uint16_t longPressMs);
void        DevBtn_EventAttach(void* btn, void (*cb)(void* btn, int event));
void        DevBtn_EventMonitor(void* btn, bool isPress);

void        DevTone_SetCallback(void (*cb)(uint32_t freq, uint16_t volume));
void        DevTone_Update(uint32_t tick);
void        DevTone_Play(const TonePlayer_MusicNode_t* music, uint16_t length);

bool        DevSD_Begin(int csPin);
uint32_t    DevSD_CardSize(void);
uint8_t     DevSD_CardType(void);
int         DevSD_CardErrorCode(void);
bool        DevSD_Exists(const char* path);
bool        DevSD_Mkdir(const char* path);
const char* DevSD_GetTypeName(void);
void        DevSD_SetDateTimeCallback(void (*cb)(uint16_t* date, uint16_t* time));

#ifdef __cplusplus
}
#endif

#endif
