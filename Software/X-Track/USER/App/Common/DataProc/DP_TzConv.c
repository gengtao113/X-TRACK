#include "DataProc.h"
#include "../HAL/hal_c.h"
#include "Utils/Time/time_c.h"

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    GPS_Info_t gps;
    SysConfig_Info_t sysCfg;
    Clock_Info_t* info;

    (void)from;

    if (event != ACCOUNT_EVENT_SUB_PULL)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (size != sizeof(Clock_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    if (Account_Pull(account, "GPS", &gps, sizeof(gps)) != ACCOUNT_RES_OK)
    {
        return ACCOUNT_RES_UNKNOW;
    }

    if (Account_Pull(account, "SysConfig", &sysCfg, sizeof(sysCfg)) != ACCOUNT_RES_OK)
    {
        return ACCOUNT_RES_UNKNOW;
    }

    Time_Set(
        gps.clock.hour,
        gps.clock.minute,
        gps.clock.second,
        gps.clock.day,
        gps.clock.month,
        gps.clock.year
    );
    Time_Adjust(sysCfg.timeZone * TIME_SECS_PER_HOUR);

    info = (Clock_Info_t*)data;
    info->year = Time_Year();
    info->month = Time_Month();
    info->day = Time_Day();
    info->hour = Time_Hour();
    info->minute = Time_Minute();
    info->second = Time_Second();

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(TzConv)
{
    Account_Subscribe(account, "GPS");
    Account_Subscribe(account, "SysConfig");
    Account_SetCallback(account, onEvent);
}
