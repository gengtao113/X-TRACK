#include "DataProc.h"
#include "Utils/Filters/Filters.h"
#include "../HAL/HAL.h"

static void onTimer(Account* account)
{
    static bool lastStatus = false;

    HAL::Power_Info_t power;
    HAL::Power_GetInfo(&power);
    if (power.isCharging != lastStatus)
    {
        DataProc::MusicPlayer_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.music = power.isCharging ? "BattChargeStart" : "BattChargeEnd";
        Account_Notify(account, "MusicPlayer", &info, sizeof(info));

        lastStatus = power.isCharging;
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    static Filter::Hysteresis<int16_t>      battUsageHysFilter(2);
    static Filter::MedianQueue<int16_t, 10> battUsageMqFilter;

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

    if (size != sizeof(HAL::Power_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    HAL::Power_Info_t powerInfo;
    HAL::Power_GetInfo(&powerInfo);

    int16_t usage = powerInfo.usage;
    usage = battUsageHysFilter.GetNext(usage);
    usage = battUsageMqFilter.GetNext(usage);
    powerInfo.usage = (uint8_t)usage;

    memcpy(data, &powerInfo, size);

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(Power)
{
    Account_Subscribe(account, "MusicPlayer");
    Account_SetCallback(account, onEvent);
    Account_SetTimerPeriod(account, 500);
}
