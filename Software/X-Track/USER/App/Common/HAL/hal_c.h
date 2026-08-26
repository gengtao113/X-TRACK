#ifndef __HAL_C_H
#define __HAL_C_H

#include "HAL_Def.h"

#ifdef __cplusplus
#define HALC_GPS_INFO   HAL::GPS_Info_t
#define HALC_CLOCK_INFO HAL::Clock_Info_t
#else
#define HALC_GPS_INFO   GPS_Info_t
#define HALC_CLOCK_INFO Clock_Info_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*HAL_CommitFunc_t)(void* info, void* userData);

bool   HAL_GPS_GetInfo(HALC_GPS_INFO* info);
double HAL_GPS_GetDistanceOffset(HALC_GPS_INFO* info, double preLong, double preLat);
void   HAL_IMU_SetCommitCallback(HAL_CommitFunc_t func, void* userData);
void   HAL_MAG_SetCommitCallback(HAL_CommitFunc_t func, void* userData);
bool   HAL_Audio_PlayMusic(const char* name);
void   HAL_Clock_GetInfo(HALC_CLOCK_INFO* info);
void   HAL_Clock_SetInfo(const HALC_CLOCK_INFO* info);
void   HAL_Buzz_SetEnable(bool en);

#ifdef __cplusplus
}
#endif

#endif
