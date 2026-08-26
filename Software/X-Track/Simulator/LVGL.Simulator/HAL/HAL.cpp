#include "HAL.h"

void HAL_Init()
{
    Buzz_init();
    Audio_Init();
    GPS_Init();
}

void HAL_Update()
{
    IMU_Update();
    MAG_Update();
    Audio_Update();
}
