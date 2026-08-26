#include "DataProc.h"
#include "../HAL/hal_c.h"
#include "Config/Config.h"

typedef enum
{
    GPS_STATUS_DISCONNECT,
    GPS_STATUS_UNSTABLE,
    GPS_STATUS_CONNECT,
} GPS_Status_t;

static void onTimer(Account* account)
{
    GPS_Info_t gpsInfo;
    HAL_GPS_GetInfo(&gpsInfo);

    int satellites = gpsInfo.satellites;

    static GPS_Status_t nowStatus = GPS_STATUS_DISCONNECT;
    static GPS_Status_t lastStatus = GPS_STATUS_DISCONNECT;

    if (satellites > 7)
    {
        nowStatus = GPS_STATUS_CONNECT;
    }
    else if (satellites < 5 && satellites >= 3)
    {
        nowStatus = GPS_STATUS_UNSTABLE;
    }
    else if (satellites == 0)
    {
        nowStatus = GPS_STATUS_DISCONNECT;
    }

    if (nowStatus != lastStatus)
    {
        const char* music[] =
        {
            "Disconnect",
            "UnstableConnect",
            "Connect"
        };

        MusicPlayer_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.music = music[nowStatus];
        Account_Notify(account, "MusicPlayer", &info, sizeof(info));
        lastStatus = nowStatus;
    }

    if (satellites >= 3)
    {
        Account_Commit(account, &gpsInfo, sizeof(gpsInfo));
        Account_Publish(account);
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    (void)from;

    if (event == ACCOUNT_EVENT_TIMER)
    {
        onTimer(account);
        return ACCOUNT_RES_OK;
    }

    if (event != ACCOUNT_EVENT_SUB_PULL)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (size != sizeof(GPS_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    HAL_GPS_GetInfo((GPS_Info_t*)data);

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(GPS)
{
    Account_Subscribe(account, "MusicPlayer");

    Account_SetCallback(account, onEvent);
    Account_SetTimerPeriod(account, CONFIG_GPS_REFR_PERIOD);
}
