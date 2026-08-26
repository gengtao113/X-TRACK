#include "LiveMapModel.h"
#include "Config/Config.h"
#include "Common/DataProc/DataProc.h"
#include "Utils/MapConv/MapConv.h"
#include "Utils/TileConv/TileConv.h"
#include "Utils/TrackFilter/TrackFilter.h"
#include "Utils/PointContainer/PointContainer.h"
#include <string.h>

struct LiveMapModelCxx
{
    MapConv mapConv;
    TileConv tileConv;
    TrackPointFilter pointFilter;
    TrackLineFilter lineFilter;
    Account* account;
    LiveMapModel* owner;
    LiveMap_PointCb point_cb;
    void* point_user;
    LiveMap_LineCb line_cb;
    void* line_user;
};

static LiveMapModelCxx* cxx_of(LiveMapModel* m)
{
    return (LiveMapModelCxx*)m->cxx;
}

static void on_point_out(TrackPointFilter* filter, const TrackPointFilter::Point_t* point)
{
    LiveMapModelCxx* cxx = (LiveMapModelCxx*)filter->userData;
    if (cxx->point_cb)
    {
        cxx->point_cb(cxx->point_user, (int32_t)point->x, (int32_t)point->y);
    }
}

static void on_line_event(TrackLineFilter* filter, TrackLineFilter::Event_t* event)
{
    LiveMapModelCxx* cxx = (LiveMapModelCxx*)filter->userData;
    LiveMapLineEvent_t out;

    if (!cxx->line_cb)
    {
        return;
    }

    out.code = (int)event->code;
    out.has_point = (event->point != nullptr) ? 1 : 0;
    out.x = event->point ? event->point->x : 0;
    out.y = event->point ? event->point->y : 0;
    cxx->line_cb(cxx->line_user, &out);
}

static int on_account_event(Account* account, Account::EventParam_t* param)
{
    LiveMapModel* owner;

    if (param->event != Account::EVENT_PUB_PUBLISH)
    {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (strcmp(param->tran->ID, "SportStatus") != 0
            || param->size != sizeof(HAL::SportStatus_Info_t))
    {
        return Account::RES_PARAM_ERROR;
    }

    owner = (LiveMapModel*)account->UserData;
    memcpy(&(owner->sportStatusInfo), param->data_p, param->size);
    return Account::RES_OK;
}

void LiveMapModel_Construct(LiveMapModel* m)
{
    LiveMapModelCxx* cxx = new LiveMapModelCxx();
    cxx->account = nullptr;
    cxx->owner = m;
    cxx->point_cb = nullptr;
    cxx->point_user = nullptr;
    cxx->line_cb = nullptr;
    cxx->line_user = nullptr;
    m->cxx = cxx;
    memset(&m->sportStatusInfo, 0, sizeof(m->sportStatusInfo));
}

void LiveMapModel_Destruct(LiveMapModel* m)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    if (!cxx)
    {
        return;
    }
    LiveMapModel_Deinit(m);
    delete cxx;
    m->cxx = nullptr;
}

void LiveMapModel_Init(LiveMapModel* m)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    cxx->account = new Account("LiveMapModel", DataProc::Center(), 0, m);
    cxx->account->Subscribe("GPS");
    cxx->account->Subscribe("SportStatus");
    cxx->account->Subscribe("TrackFilter");
    cxx->account->Subscribe("SysConfig");
    cxx->account->Subscribe("StatusBar");
    cxx->account->SetEventCallback(on_account_event);
}

void LiveMapModel_Deinit(LiveMapModel* m)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    if (cxx && cxx->account)
    {
        delete cxx->account;
        cxx->account = nullptr;
    }
}

void LiveMapModel_TileSetSize(LiveMapModel* m, uint32_t size)
{
    cxx_of(m)->tileConv.SetTileSize(size);
}

void LiveMapModel_TileSetViewSize(LiveMapModel* m, uint32_t w, uint32_t h)
{
    cxx_of(m)->tileConv.SetViewSize(w, h);
}

void LiveMapModel_TileSetFocus(LiveMapModel* m, int32_t x, int32_t y)
{
    cxx_of(m)->tileConv.SetFocusPos(x, y);
}

uint32_t LiveMapModel_TileGetContainer(LiveMapModel* m, LiveMapRect_t* rect)
{
    TileConv::Rect_t r;
    uint32_t n = cxx_of(m)->tileConv.GetTileContainer(&r);
    rect->x = r.x;
    rect->y = r.y;
    rect->width = r.width;
    rect->height = r.height;
    return n;
}

void LiveMapModel_TileGetOffset(LiveMapModel* m, LiveMapPoint_t* offset, const LiveMapPoint_t* point)
{
    TileConv::Point_t o;
    TileConv::Point_t p = { point->x, point->y };
    cxx_of(m)->tileConv.GetOffset(&o, &p);
    offset->x = o.x;
    offset->y = o.y;
}

void LiveMapModel_TileGetFocusOffset(LiveMapModel* m, LiveMapPoint_t* offset)
{
    TileConv::Point_t o;
    cxx_of(m)->tileConv.GetFocusOffset(&o);
    offset->x = o.x;
    offset->y = o.y;
}

void LiveMapModel_TileGetContainerOffset(LiveMapModel* m, LiveMapPoint_t* offset)
{
    TileConv::Point_t o;
    cxx_of(m)->tileConv.GetTileContainerOffset(&o);
    offset->x = o.x;
    offset->y = o.y;
}

void LiveMapModel_TileGetPos(LiveMapModel* m, uint32_t index, LiveMapPoint_t* pos)
{
    TileConv::Point_t p;
    cxx_of(m)->tileConv.GetTilePos(index, &p);
    pos->x = p.x;
    pos->y = p.y;
}

int16_t LiveMapModel_MapGetLevel(LiveMapModel* m)
{
    return cxx_of(m)->mapConv.GetLevel();
}

int16_t LiveMapModel_MapGetLevelMin(void)
{
    return MapConv::GetLevelMin();
}

int16_t LiveMapModel_MapGetLevelMax(void)
{
    return MapConv::GetLevelMax();
}

void LiveMapModel_MapSetLevel(LiveMapModel* m, int16_t level)
{
    cxx_of(m)->mapConv.SetLevel(level);
}

void LiveMapModel_MapConvertCoord(LiveMapModel* m, double lon, double lat, int32_t* x, int32_t* y)
{
    cxx_of(m)->mapConv.ConvertMapCoordinate(lon, lat, x, y);
}

void LiveMapModel_MapConvertPath(LiveMapModel* m, int32_t x, int32_t y, char* path, uint32_t len)
{
    cxx_of(m)->mapConv.ConvertMapPath(x, y, path, len);
}

void LiveMapModel_PointFilterSetThreshold(LiveMapModel* m, double th)
{
    cxx_of(m)->pointFilter.SetOffsetThreshold(th);
}

void LiveMapModel_PointFilterSetCallback(LiveMapModel* m, LiveMap_PointCb cb, void* user)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    cxx->point_cb = cb;
    cxx->point_user = user;
    cxx->pointFilter.SetOutputPointCallback(on_point_out);
    cxx->pointFilter.userData = cxx;
}

void LiveMapModel_PointFilterClearCallback(LiveMapModel* m)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    cxx->point_cb = nullptr;
    cxx->pointFilter.SetOutputPointCallback(nullptr);
}

void LiveMapModel_PointFilterPush(LiveMapModel* m, int32_t x, int32_t y)
{
    cxx_of(m)->pointFilter.PushPoint(x, y);
}

void LiveMapModel_LineFilterSetCallback(LiveMapModel* m, LiveMap_LineCb cb, void* user)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    cxx->line_cb = cb;
    cxx->line_user = user;
    cxx->lineFilter.SetOutputPointCallback(on_line_event);
    cxx->lineFilter.userData = cxx;
}

void LiveMapModel_LineFilterSetClipArea(LiveMapModel* m, const LiveMapArea_t* area)
{
    TrackLineFilter::Area_t a = { area->x0, area->y0, area->x1, area->y1 };
    cxx_of(m)->lineFilter.SetClipArea(&a);
}

void LiveMapModel_LineFilterReset(LiveMapModel* m)
{
    cxx_of(m)->lineFilter.Reset();
}

void LiveMapModel_LineFilterPush(LiveMapModel* m, int32_t x, int32_t y)
{
    cxx_of(m)->lineFilter.PushPoint(x, y);
}

void LiveMapModel_LineFilterPushEnd(LiveMapModel* m)
{
    cxx_of(m)->lineFilter.PushEnd();
}

void LiveMapModel_GetGPS_Info(LiveMapModel* m, GPS_Info_t* info)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    memset(info, 0, sizeof(*info));
    if (cxx->account->Pull("GPS", info, sizeof(*info)) != Account::RES_OK)
    {
        return;
    }

    if (!info->isVaild)
    {
        DataProc::SysConfig_Info_t sysConfig;
        if (cxx->account->Pull("SysConfig", &sysConfig, sizeof(sysConfig)) == Account::RES_OK)
        {
            info->longitude = sysConfig.longitude;
            info->latitude = sysConfig.latitude;
        }
    }
}

void LiveMapModel_GetArrowTheme(LiveMapModel* m, char* buf, uint32_t size)
{
    DataProc::SysConfig_Info_t sysConfig;
    if (cxx_of(m)->account->Pull("SysConfig", &sysConfig, sizeof(sysConfig)) != Account::RES_OK)
    {
        buf[0] = '\0';
        return;
    }
    strncpy(buf, sysConfig.arrowTheme, size);
    buf[size - 1] = '\0';
}

bool LiveMapModel_GetTrackFilterActive(LiveMapModel* m)
{
    DataProc::TrackFilter_Info_t info;
    if (cxx_of(m)->account->Pull("TrackFilter", &info, sizeof(info)) != Account::RES_OK)
    {
        return false;
    }
    return info.isActive;
}

struct LiveMapReloadWrap
{
    LiveMap_PointCb cb;
    void* user;
};

static void on_reload_point(TrackPointFilter* filter, const TrackPointFilter::Point_t* point)
{
    LiveMapReloadWrap* w = (LiveMapReloadWrap*)filter->userData;
    if (w->cb)
    {
        w->cb(w->user, (int32_t)point->x, (int32_t)point->y);
    }
}

void LiveMapModel_TrackReload(LiveMapModel* m, LiveMap_PointCb cb, void* user)
{
    LiveMapModelCxx* cxx = cxx_of(m);
    DataProc::TrackFilter_Info_t info;
    PointContainer* pointContainer;
    TrackPointFilter ptFilter;
    LiveMapReloadWrap wrap;
    int32_t pointX;
    int32_t pointY;

    if (cxx->account->Pull("TrackFilter", &info, sizeof(info)) != Account::RES_OK)
    {
        return;
    }

    if (!info.isActive || info.pointCont == nullptr)
    {
        return;
    }

    pointContainer = (PointContainer*)info.pointCont;
    pointContainer->PopStart();
    cxx->pointFilter.Reset();

    wrap.cb = cb;
    wrap.user = user;
    ptFilter.SetOffsetThreshold(CONFIG_TRACK_FILTER_OFFSET_THRESHOLD);
    ptFilter.SetOutputPointCallback(on_reload_point);
    ptFilter.SetSecondFilterModeEnable(true);
    ptFilter.userData = &wrap;

    while (pointContainer->PopPoint(&pointX, &pointY))
    {
        int32_t mapX;
        int32_t mapY;
        cxx->mapConv.ConvertMapLevelPos(
            &mapX, &mapY,
            pointX, pointY,
            info.level
        );
        ptFilter.PushPoint(mapX, mapY);
    }
    ptFilter.PushEnd();
}

void LiveMapModel_SetStatusBarStyle(LiveMapModel* m, StatusBar_Style_t style)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.cmd = DataProc::STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;
    cxx_of(m)->account->Notify("StatusBar", &info, sizeof(info));
}
