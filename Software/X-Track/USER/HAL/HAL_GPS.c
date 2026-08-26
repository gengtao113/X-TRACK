#include "HAL.h"
#include "hal_dev.h"
#include <string.h>

#define GPS_USE_TRANSPARENT    CONFIG_GPS_USE_TRANSPARENT

void GPS_Init(void)
{
    Serial2_Begin(9600);

    Serial_Print("GPS: TinyGPS++ library v. ");
    Serial_Print(DevGPS_LibraryVersion());
    Serial_Println(" by Mikal Hart");
}

void GPS_Update(void)
{
#if CONFIG_GPS_BUF_OVERLOAD_CHK && !GPS_USE_TRANSPARENT
    int available = Serial2_Available();
    Serial_Printf("GPS: Buffer available = %d", available);
    if(available >= SERIAL_RX_BUFFER_SIZE / 2)
    {
        Serial_Print(", maybe overload!");
    }
    Serial_Println("");
#endif

    while (Serial2_Available() > 0)
    {
        char c = (char)Serial2_Read();
#if GPS_USE_TRANSPARENT
        Serial_Write((uint8_t)c);
#endif
        DevGPS_Encode(c);
    }

#if GPS_USE_TRANSPARENT
    while (Serial_Available() > 0)
    {
        Serial2_Write((uint8_t)Serial_Read());
    }
#endif
}

bool GPS_GetInfo(GPS_Info_t* info)
{
    return DevGPS_FillInfo(info);
}

bool GPS_LocationIsValid(void)
{
    return DevGPS_LocationValid();
}

double GPS_GetDistanceOffset(GPS_Info_t* info, double preLong, double preLat)
{
    return DevGPS_Distance(info->latitude, info->longitude, preLat, preLong);
}
