#include "DataProc.h"
#include "../HAL/hal_c.h"
#include "Config/Config.h"

static SysConfig_Info_t sysConfig;

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    SysConfig_Info_t* info;

    (void)from;

    if (size != sizeof(SysConfig_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    info = (SysConfig_Info_t*)data;

    switch (event)
    {
    case ACCOUNT_EVENT_NOTIFY:
    {
        if (info->cmd == SYSCONFIG_CMD_LOAD)
        {
            HAL_Buzz_SetEnable(sysConfig.soundEnable);
        }
        else if (info->cmd == SYSCONFIG_CMD_SAVE)
        {
            GPS_Info_t gpsInfo;
            if (Account_Pull(account, "GPS", &gpsInfo, sizeof(gpsInfo)) != ACCOUNT_RES_OK)
            {
                return ACCOUNT_RES_UNKNOW;
            }

            if (gpsInfo.isVaild)
            {
                sysConfig.longitude = (float)gpsInfo.longitude;
                sysConfig.latitude = (float)gpsInfo.latitude;
            }
        }
    }
    break;
    case ACCOUNT_EVENT_SUB_PULL:
    {
        memcpy(info, &sysConfig, sizeof(sysConfig));
    }
    break;
    default:
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(SysConfig)
{
    Account_Subscribe(account, "Storage");
    Account_Subscribe(account, "GPS");
    Account_SetCallback(account, onEvent);

    memset(&sysConfig, 0, sizeof(sysConfig));

#   define SYSCGF_STRCPY(dest, src) \
do{ \
    strncpy(dest, src, sizeof(dest)); \
    dest[sizeof(dest) - 1] = '\0'; \
}while(0)

    sysConfig.cmd         = SYSCONFIG_CMD_LOAD;
    sysConfig.longitude   = CONFIG_GPS_LONGITUDE_DEFAULT;
    sysConfig.latitude    = CONFIG_GPS_LATITUDE_DEFAULT;
    sysConfig.timeZone    = CONFIG_SYSTEM_TIME_ZONE_DEFAULT;
    sysConfig.soundEnable = CONFIG_SYSTEM_SOUND_ENABLE_DEFAULT;
    SYSCGF_STRCPY(sysConfig.language, CONFIG_SYSTEM_LANGUAGE_DEFAULT);
    SYSCGF_STRCPY(sysConfig.arrowTheme, CONFIG_ARROW_THEME_DEFAULT);
    SYSCGF_STRCPY(sysConfig.mapDirPath, CONFIG_MAP_DIR_PATH_DEFAULT);
    SYSCGF_STRCPY(sysConfig.mapExtName, CONFIG_MAP_EXT_NAME_DEFAULT);
    sysConfig.mapWGS84    = CONFIG_MAP_USE_WGS84_DEFAULT;

    STORAGE_VALUE_REG(account, sysConfig.longitude, STORAGE_TYPE_FLOAT);
    STORAGE_VALUE_REG(account, sysConfig.latitude, STORAGE_TYPE_FLOAT);

    STORAGE_VALUE_REG(account, sysConfig.soundEnable, STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sysConfig.timeZone, STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sysConfig.language, STORAGE_TYPE_STRING);
    STORAGE_VALUE_REG(account, sysConfig.arrowTheme, STORAGE_TYPE_STRING);
    STORAGE_VALUE_REG(account, sysConfig.mapDirPath, STORAGE_TYPE_STRING);
    STORAGE_VALUE_REG(account, sysConfig.mapExtName, STORAGE_TYPE_STRING);
    STORAGE_VALUE_REG(account, sysConfig.mapWGS84, STORAGE_TYPE_INT);
}
