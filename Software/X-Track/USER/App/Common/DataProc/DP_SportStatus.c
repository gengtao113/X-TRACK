#include "DataProc.h"
#include "../HAL/hal_c.h"
#include "Config/Config.h"
#include "dataproc_c.h"

#define CALORIC_CORFFICIENT 0.5f

static SportStatus_Info_t sportStatus;

static double SportStatus_GetDistanceOffset(GPS_Info_t* gpsInfo)
{
    static bool isFirst = true;
    static double preLongitude;
    static double preLatitude;

    double offset = 0.0f;

    if (!isFirst)
    {
        offset = HAL_GPS_GetDistanceOffset(gpsInfo, preLongitude, preLatitude);
    }
    else
    {
        isFirst = false;
    }

    preLongitude = gpsInfo->longitude;
    preLatitude = gpsInfo->latitude;

    return offset;
}

static void onTimer(Account* account)
{
    GPS_Info_t gpsInfo;
    uint32_t timeElaps;
    float speedKph;
    bool isSignalInterruption;

    if (Account_Pull(account, "GPS", &gpsInfo, sizeof(gpsInfo)) != ACCOUNT_RES_OK)
    {
        return;
    }

    timeElaps = DataProc_GetTickElaps(sportStatus.lastTick);

    speedKph = 0.0f;
    isSignalInterruption = (gpsInfo.isVaild && (gpsInfo.satellites == 0));

    if (gpsInfo.satellites >= 3)
    {
        float spd = gpsInfo.speed;
        speedKph = spd > 1 ? spd : 0;
    }

    if (speedKph > 0.0f || isSignalInterruption)
    {
        sportStatus.singleTime += timeElaps;
        sportStatus.totalTime += timeElaps;

        if (speedKph > 0.0f)
        {
            float dist = (float)SportStatus_GetDistanceOffset(&gpsInfo);

            sportStatus.singleDistance += dist;
            sportStatus.totalDistance += dist;

            float meterPerSec = sportStatus.singleDistance * 1000 / sportStatus.singleTime;
            sportStatus.speedAvgKph = meterPerSec * 3.6f;

            if (speedKph > sportStatus.speedMaxKph)
            {
                sportStatus.speedMaxKph = speedKph;
            }

            float calorie = speedKph * sportStatus.weight * CALORIC_CORFFICIENT * timeElaps / 1000 / 3600;
            sportStatus.singleCalorie += calorie;
        }
    }

    sportStatus.speedKph = speedKph;

    sportStatus.lastTick = DataProc_GetTick();
    Account_Commit(account, &sportStatus, sizeof(sportStatus));
    Account_Publish(account);
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

    if (size != sizeof(sportStatus))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    memcpy(data, &sportStatus, size);
    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(SportStatus)
{
    memset(&sportStatus, 0, sizeof(sportStatus));
    sportStatus.weight = CONFIG_WEIGHT_DEFAULT;

    Account_Subscribe(account, "GPS");
    Account_Subscribe(account, "Storage");

    STORAGE_VALUE_REG(account, sportStatus.totalDistance, STORAGE_TYPE_FLOAT);
    STORAGE_VALUE_REG(account, sportStatus.totalTimeUINT32[0], STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sportStatus.totalTimeUINT32[1], STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sportStatus.speedMaxKph, STORAGE_TYPE_FLOAT);
    STORAGE_VALUE_REG(account, sportStatus.weight, STORAGE_TYPE_FLOAT);

    sportStatus.lastTick = DataProc_GetTick();

    Account_SetCallback(account, onEvent);
    Account_SetTimerPeriod(account, 500);
}
