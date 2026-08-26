#include "DataProc.h"
#include "Utils/MapConv/mapconv_c.h"
#include "Utils/TrackFilter/track_point_filter_c.h"
#include "Utils/PointContainer/point_container_c.h"
#include "Config/Config.h"

typedef struct
{
    MapConv* mapConv;
    TrackPointFilter* pointFilter;
    void* pointContainer;
    bool isStarted;
    bool isActive;
} TrackFilter_t;

static TrackFilter_t trackFilter;

static void onNotify(Account* account, TrackFilter_Info_t* info)
{
    (void)account;

    switch (info->cmd)
    {
    case TRACK_FILTER_CMD_START:
        trackFilter.pointContainer = PointContainer_Create();
        TrackPointFilter_Reset(trackFilter.pointFilter);
        trackFilter.isActive = true;
        trackFilter.isStarted = true;
        LV_LOG_USER("Track filter start");
        break;
    case TRACK_FILTER_CMD_PAUSE:
        trackFilter.isActive = false;
        LV_LOG_USER("Track filter pause");
        break;
    case TRACK_FILTER_CMD_CONTINUE:
        trackFilter.isActive = true;
        LV_LOG_USER("Track filter continue");
        break;
    case TRACK_FILTER_CMD_STOP:
    {
        uint32_t sum = 0;
        uint32_t output = 0;

        trackFilter.isStarted = false;
        trackFilter.isActive = false;

        if (trackFilter.pointContainer)
        {
            PointContainer_Destroy(trackFilter.pointContainer);
            trackFilter.pointContainer = NULL;
        }

        TrackPointFilter_GetCounts(trackFilter.pointFilter, &sum, &output);
        LV_LOG_USER(
            "Track filter stop, filted(%d%%): sum = %d, output = %d",
            sum ? (100 - output * 100 / sum) : 0,
            sum,
            output
        );
        break;
    }
    default:
        break;
    }
}

static void onPublish(Account* account, GPS_Info_t* gps)
{
    int32_t mapX;
    int32_t mapY;

    (void)account;

    MapConv_ConvertMapCoordinate(
        trackFilter.mapConv,
        gps->longitude,
        gps->latitude,
        &mapX,
        &mapY
    );

    if (TrackPointFilter_PushPoint(trackFilter.pointFilter, mapX, mapY))
    {
        PointContainer_PushPoint(trackFilter.pointContainer, mapX, mapY);
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    (void)from;

    if (event == ACCOUNT_EVENT_PUB_PUBLISH
            && size == sizeof(GPS_Info_t))
    {
        if (trackFilter.isActive)
        {
            onPublish(account, (GPS_Info_t*)data);
        }

        return ACCOUNT_RES_OK;
    }

    if (size != sizeof(TrackFilter_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    switch (event)
    {
    case ACCOUNT_EVENT_SUB_PULL:
    {
        TrackFilter_Info_t* info = (TrackFilter_Info_t*)data;
        info->pointCont = trackFilter.pointContainer;
        info->level = (uint8_t)MapConv_GetLevel(trackFilter.mapConv);
        info->isActive = trackFilter.isStarted;
        break;
    }
    case ACCOUNT_EVENT_NOTIFY:
        onNotify(account, (TrackFilter_Info_t*)data);
        break;

    default:
        break;
    }

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(TrackFilter)
{
    Account_Subscribe(account, "GPS");
    Account_SetCallback(account, onEvent);

    trackFilter.mapConv = MapConv_Create();
    trackFilter.pointFilter = TrackPointFilter_Create();
    trackFilter.pointContainer = NULL;
    trackFilter.isActive = false;
    trackFilter.isStarted = false;

    MapConv_SetLevel(trackFilter.mapConv, CONFIG_LIVE_MAP_LEVEL_DEFAULT);
    TrackPointFilter_SetOffsetThreshold(trackFilter.pointFilter, CONFIG_TRACK_FILTER_OFFSET_THRESHOLD);
}
