#include "hal_c.h"
#include "HAL.h"

bool HAL_GPS_GetInfo(HALC_GPS_INFO* info)
{
    return HAL::GPS_GetInfo(info);
}

double HAL_GPS_GetDistanceOffset(HALC_GPS_INFO* info, double preLong, double preLat)
{
    return HAL::GPS_GetDistanceOffset(info, preLong, preLat);
}

void HAL_IMU_SetCommitCallback(HAL_CommitFunc_t func, void* userData)
{
    HAL::IMU_SetCommitCallback(reinterpret_cast<HAL::CommitFunc_t>(func), userData);
}

void HAL_MAG_SetCommitCallback(HAL_CommitFunc_t func, void* userData)
{
    HAL::MAG_SetCommitCallback(reinterpret_cast<HAL::CommitFunc_t>(func), userData);
}

bool HAL_Audio_PlayMusic(const char* name)
{
    return HAL::Audio_PlayMusic(name);
}

void HAL_Clock_GetInfo(HALC_CLOCK_INFO* info)
{
    HAL::Clock_GetInfo(info);
}

void HAL_Clock_SetInfo(const HALC_CLOCK_INFO* info)
{
    HAL::Clock_SetInfo(info);
}

void HAL_Buzz_SetEnable(bool en)
{
    HAL::Buzz_SetEnable(en);
}
