#include "HAL.h"
#include "Config/Config.h"
#include "hal_dev.h"

#ifndef FAT_DATE
#define FAT_DATE(year, month, day) ((uint16_t)(((year) - 1980) << 9 | (month) << 5 | (day)))
#endif
#ifndef FAT_TIME
#define FAT_TIME(hour, minute, second) ((uint16_t)((hour) << 11 | (minute) << 5 | ((second) >> 1)))
#endif

static bool SD_IsReady = false;
static uint32_t SD_CardSize = 0;

static SD_CallbackFunction_t SD_EventCallback = NULL;

static void SD_GetDateTime(uint16_t* date, uint16_t* time)
{
    Clock_Info_t clock;
    Clock_GetInfo(&clock);

    *date = FAT_DATE(clock.year, clock.month, clock.day);
    *time = FAT_TIME(clock.hour, clock.minute, clock.second);
}

static bool SD_CheckDir(const char* path)
{
    bool retval = true;
    if(!DevSD_Exists(path))
    {
        Serial_Printf("SD: Auto create path \"%s\"...", path);
        retval = DevSD_Mkdir(path);
        Serial_Println(retval ? "success" : "failed");
    }
    return retval;
}

bool SD_Init(void)
{
    bool retval = true;

    pinMode(CONFIG_SD_CD_PIN, INPUT_PULLUP);
    if(digitalRead(CONFIG_SD_CD_PIN))
    {
        Serial_Println("SD: CARD was not inserted");
        retval = false;
    }

    Serial_Print("SD: init...");
    retval = DevSD_Begin(CONFIG_SD_CS_PIN);

    if(retval)
    {
        SD_CardSize = DevSD_CardSize();
        DevSD_SetDateTimeCallback(SD_GetDateTime);
        SD_CheckDir(CONFIG_TRACK_RECORD_FILE_DIR_NAME);
        Serial_Printf(
            "success, Type: %s, Size: %0.2f GB\r\n",
            SD_GetTypeName(),
            SD_GetCardSizeMB() / 1024.0f
        );
    }
    else
    {
        Serial_Printf("failed: 0x%x\r\n", DevSD_CardErrorCode());
    }

    SD_IsReady = retval;

    return retval;
}

bool SD_GetReady(void)
{
    return SD_IsReady;
}

float SD_GetCardSizeMB(void)
{
#   define CONV_MB(size) (size*0.000512f)
    return CONV_MB(SD_CardSize);
}

const char* SD_GetTypeName(void)
{
    if(!SD_CardSize)
    {
        return "Unknown";
    }
    return DevSD_GetTypeName();
}

static void SD_Check(bool isInsert)
{
    if(isInsert)
    {
        bool ret = SD_Init();

        if(ret && SD_EventCallback)
        {
            SD_EventCallback(true);
        }

        Audio_PlayMusic(ret ? "DeviceInsert" : "Error");
    }
    else
    {
        SD_IsReady = false;

        if(SD_EventCallback)
        {
            SD_EventCallback(false);
            SD_CardSize = 0;
        }

        Audio_PlayMusic("DevicePullout");
    }
}

void SD_SetEventCallback(SD_CallbackFunction_t callback)
{
    SD_EventCallback = callback;
}

void SD_Update(void)
{
    bool isInsert = (digitalRead(CONFIG_SD_CD_PIN) == LOW);

    CM_VALUE_MONITOR(isInsert, SD_Check(isInsert));
}
