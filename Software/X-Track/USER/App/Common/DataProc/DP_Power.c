#include "DataProc.h"
#include "../HAL/hal_c.h"
#include <stdlib.h>

#define POWER_HYS_VAL   2
#define POWER_MQ_SIZE   10

typedef struct
{
    int16_t lastValue;
    int16_t hysValue;
} PowerHys_t;

typedef struct
{
    int16_t buffer[POWER_MQ_SIZE];
    int16_t bufferSort[POWER_MQ_SIZE];
    int16_t lastValue;
    size_t dataIndex;
    bool isFirst;
} PowerMq_t;

static PowerHys_t battUsageHysFilter;
static PowerMq_t battUsageMqFilter;

static int16_t PowerHys_GetNext(PowerHys_t* f, int16_t value)
{
    int16_t diff = (int16_t)(value - f->lastValue);
    if (diff < 0)
    {
        diff = (int16_t)(-diff);
    }
    if (diff > f->hysValue)
    {
        f->lastValue = value;
    }
    return f->lastValue;
}

static int PowerMq_Cmp(const void* a, const void* b)
{
    int16_t da = *(const int16_t*)a;
    int16_t db = *(const int16_t*)b;
    return (da > db) - (da < db);
}

static bool PowerMq_FillBuffer(PowerMq_t* f, int16_t value)
{
    if (f->dataIndex < POWER_MQ_SIZE)
    {
        f->buffer[f->dataIndex] = value;
        f->dataIndex++;
        return false;
    }
    return true;
}

static int16_t PowerMq_GetNext(PowerMq_t* f, int16_t value)
{
    size_t i;

    if (f->isFirst)
    {
        f->isFirst = !PowerMq_FillBuffer(f, value);
        f->lastValue = value;
    }
    else
    {
        f->dataIndex %= POWER_MQ_SIZE;
        f->buffer[f->dataIndex] = value;
        f->dataIndex++;

        for (i = 0; i < POWER_MQ_SIZE; i++)
        {
            f->bufferSort[i] = f->buffer[i];
        }
        qsort(f->bufferSort, POWER_MQ_SIZE, sizeof(int16_t), PowerMq_Cmp);
        f->lastValue = f->bufferSort[POWER_MQ_SIZE / 2];
    }

    return f->lastValue;
}

static void onTimer(Account* account)
{
    static bool lastStatus = false;
    Power_Info_t power;

    HAL_Power_GetInfo(&power);
    if (power.isCharging != lastStatus)
    {
        MusicPlayer_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.music = power.isCharging ? "BattChargeStart" : "BattChargeEnd";
        Account_Notify(account, "MusicPlayer", &info, sizeof(info));

        lastStatus = power.isCharging;
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    Power_Info_t powerInfo;
    int16_t usage;

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

    if (size != sizeof(Power_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    HAL_Power_GetInfo(&powerInfo);

    usage = powerInfo.usage;
    usage = PowerHys_GetNext(&battUsageHysFilter, usage);
    usage = PowerMq_GetNext(&battUsageMqFilter, usage);
    powerInfo.usage = (uint8_t)usage;

    memcpy(data, &powerInfo, size);

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(Power)
{
    battUsageHysFilter.hysValue = POWER_HYS_VAL;
    battUsageHysFilter.lastValue = 0;
    battUsageMqFilter.isFirst = true;
    battUsageMqFilter.dataIndex = 0;

    Account_Subscribe(account, "MusicPlayer");
    Account_SetCallback(account, onEvent);
    Account_SetTimerPeriod(account, 500);
}
