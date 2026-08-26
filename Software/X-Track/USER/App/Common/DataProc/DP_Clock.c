#include "DataProc.h"
#include "../HAL/hal_c.h"

static bool Clock_Calibrate(Account* account, GPS_Info_t* gpsInfo)
{
    bool retval = false;
    if (gpsInfo->isVaild)
    {
        Clock_Info_t clock;
        if (Account_Pull(account, "TzConv", &clock, sizeof(clock)) == ACCOUNT_RES_OK)
        {
            HAL_Clock_SetInfo(&clock);
            retval = true;
        }
    }
    return retval;
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    (void)from;

    if (event == ACCOUNT_EVENT_PUB_PUBLISH)
    {
        if (size == sizeof(GPS_Info_t))
        {
            if (Clock_Calibrate(account, (GPS_Info_t*)data))
            {
                Account_Unsubscribe(account, "GPS");
            }
        }

        return ACCOUNT_RES_OK;
    }

    if (event != ACCOUNT_EVENT_SUB_PULL)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (size != sizeof(Clock_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    HAL_Clock_GetInfo((Clock_Info_t*)data);

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(Clock)
{
    Account_Subscribe(account, "TzConv");
    Account_Subscribe(account, "GPS");
    Account_SetCallback(account, onEvent);
}
