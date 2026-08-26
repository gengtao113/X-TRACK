#include "hal_dev.h"
#include "HAL_Config.h"
#include "LSM6DSM/LSM6DSM.h"
#include "LIS3MDL/LIS3MDL.h"
#include "TinyGPSPlus/src/TinyGPS++.h"
#include "ButtonEvent/ButtonEvent.h"
#include "App/Utils/TonePlayer/TonePlayer.h"
#include "SdFat.h"
#include <string.h>

static LSM6DSM imu;
static LIS3MDL mag;
static TinyGPSPlus gps;
static TonePlayer player;
static SdFat SD(&CONFIG_SD_SPI);

bool DevIMU_Init(void)
{
    return imu.Init();
}

void DevIMU_GetMotion(DEV_IMU_INFO* info)
{
    imu.GetMotion6(
        &info->ax, &info->ay, &info->az,
        &info->gx, &info->gy, &info->gz
    );
    info->steps = imu.GetCurrentStep();
}

bool DevMAG_Init(void)
{
    bool success = mag.init();
    if (success)
    {
        mag.enableDefault();
    }
    return success;
}

void DevMAG_Read(DEV_MAG_INFO* info)
{
    mag.read();
    info->x = mag.m.x;
    info->y = mag.m.y;
    info->z = mag.m.z;
}

const char* DevGPS_LibraryVersion(void)
{
    return TinyGPSPlus::libraryVersion();
}

void DevGPS_Encode(char c)
{
    gps.encode(c);
}

bool DevGPS_FillInfo(DEV_GPS_INFO* info)
{
    memset(info, 0, sizeof(*info));
    info->isVaild = gps.location.isValid();
    info->longitude = gps.location.lng();
    info->latitude = gps.location.lat();
    info->altitude = gps.altitude.meters();
    info->speed = gps.speed.kmph();
    info->course = gps.course.deg();
    info->clock.year = gps.date.year();
    info->clock.month = gps.date.month();
    info->clock.day = gps.date.day();
    info->clock.hour = gps.time.hour();
    info->clock.minute = gps.time.minute();
    info->clock.second = gps.time.second();
    info->satellites = gps.satellites.value();
    return info->isVaild;
}

bool DevGPS_LocationValid(void)
{
    return gps.location.isValid();
}

double DevGPS_Distance(double lat1, double lon1, double lat2, double lon2)
{
    return TinyGPSPlus::distanceBetween(lat1, lon1, lat2, lon2);
}

static void (*s_btn_cb)(void* btn, int event);

static void DevBtn_Trampoline(ButtonEvent* btn, int event)
{
    if (s_btn_cb)
    {
        s_btn_cb(btn, event);
    }
}

void* DevBtn_Create(uint16_t longPressMs)
{
    return new ButtonEvent(longPressMs);
}

void DevBtn_EventAttach(void* btn, void (*cb)(void* btn, int event))
{
    s_btn_cb = cb;
    ((ButtonEvent*)btn)->EventAttach(DevBtn_Trampoline);
}

void DevBtn_EventMonitor(void* btn, bool isPress)
{
    ((ButtonEvent*)btn)->EventMonitor(isPress);
}

void DevTone_SetCallback(void (*cb)(uint32_t freq, uint16_t volume))
{
    player.SetCallback(cb);
}

void DevTone_Update(uint32_t tick)
{
    player.Update(tick);
}

void DevTone_Play(const TonePlayer_Node_t* music, uint16_t length)
{
    player.Play(music, length);
}

bool DevSD_Begin(int csPin)
{
    return SD.begin(csPin, SD_SCK_MHZ(30));
}

uint32_t DevSD_CardSize(void)
{
    return SD.card()->cardSize();
}

uint8_t DevSD_CardType(void)
{
    return SD.card()->type();
}

int DevSD_CardErrorCode(void)
{
    return SD.cardErrorCode();
}

bool DevSD_Exists(const char* path)
{
    return SD.exists(path);
}

bool DevSD_Mkdir(const char* path)
{
    return SD.mkdir(path);
}

const char* DevSD_GetTypeName(void)
{
    const char* type = "Unknown";
    uint32_t size = SD.card()->cardSize();
    if (!size)
    {
        return type;
    }
    switch (SD.card()->type())
    {
    case SD_CARD_TYPE_SD1:
        type = "SD1";
        break;
    case SD_CARD_TYPE_SD2:
        type = "SD2";
        break;
    case SD_CARD_TYPE_SDHC:
        type = (size < 70000000) ? "SDHC" : "SDXC";
        break;
    default:
        break;
    }
    return type;
}

void DevSD_SetDateTimeCallback(void (*cb)(uint16_t* date, uint16_t* time))
{
    SdFile::dateTimeCallback(cb);
}
