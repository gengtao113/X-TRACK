#ifndef __LIVEMAP_MODEL_H
#define __LIVEMAP_MODEL_H

#include "Common/HAL/HAL_Def.h"
#include "Common/DataProc/DataProc_Def.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
typedef HAL::GPS_Info_t GPS_Info_t;
typedef HAL::SportStatus_Info_t SportStatus_Info_t;
typedef DataProc::StatusBar_Style_t StatusBar_Style_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int32_t x;
    int32_t y;
} LiveMapPoint_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} LiveMapRect_t;

typedef struct
{
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
} LiveMapArea_t;

enum
{
    LIVEMAP_LINE_EVENT_START_LINE = 0,
    LIVEMAP_LINE_EVENT_APPEND_POINT,
    LIVEMAP_LINE_EVENT_END_LINE,
    LIVEMAP_LINE_EVENT_RESET
};

typedef struct
{
    int code;
    int32_t x;
    int32_t y;
    int has_point;
} LiveMapLineEvent_t;

typedef void (*LiveMap_PointCb)(void* user, int32_t x, int32_t y);
typedef void (*LiveMap_LineCb)(void* user, const LiveMapLineEvent_t* ev);

typedef struct
{
    SportStatus_Info_t sportStatusInfo;
    void* cxx;
} LiveMapModel;

void     LiveMapModel_Construct(LiveMapModel* m);
void     LiveMapModel_Destruct(LiveMapModel* m);
void     LiveMapModel_Init(LiveMapModel* m);
void     LiveMapModel_Deinit(LiveMapModel* m);

void     LiveMapModel_TileSetSize(LiveMapModel* m, uint32_t size);
void     LiveMapModel_TileSetViewSize(LiveMapModel* m, uint32_t w, uint32_t h);
void     LiveMapModel_TileSetFocus(LiveMapModel* m, int32_t x, int32_t y);
uint32_t LiveMapModel_TileGetContainer(LiveMapModel* m, LiveMapRect_t* rect);
void     LiveMapModel_TileGetOffset(LiveMapModel* m, LiveMapPoint_t* offset, const LiveMapPoint_t* point);
void     LiveMapModel_TileGetFocusOffset(LiveMapModel* m, LiveMapPoint_t* offset);
void     LiveMapModel_TileGetContainerOffset(LiveMapModel* m, LiveMapPoint_t* offset);
void     LiveMapModel_TileGetPos(LiveMapModel* m, uint32_t index, LiveMapPoint_t* pos);

int16_t  LiveMapModel_MapGetLevel(LiveMapModel* m);
int16_t  LiveMapModel_MapGetLevelMin(void);
int16_t  LiveMapModel_MapGetLevelMax(void);
void     LiveMapModel_MapSetLevel(LiveMapModel* m, int16_t level);
void     LiveMapModel_MapConvertCoord(LiveMapModel* m, double lon, double lat, int32_t* x, int32_t* y);
void     LiveMapModel_MapConvertPath(LiveMapModel* m, int32_t x, int32_t y, char* path, uint32_t len);

void     LiveMapModel_PointFilterSetThreshold(LiveMapModel* m, double th);
void     LiveMapModel_PointFilterSetCallback(LiveMapModel* m, LiveMap_PointCb cb, void* user);
void     LiveMapModel_PointFilterClearCallback(LiveMapModel* m);
void     LiveMapModel_PointFilterPush(LiveMapModel* m, int32_t x, int32_t y);

void     LiveMapModel_LineFilterSetCallback(LiveMapModel* m, LiveMap_LineCb cb, void* user);
void     LiveMapModel_LineFilterSetClipArea(LiveMapModel* m, const LiveMapArea_t* area);
void     LiveMapModel_LineFilterReset(LiveMapModel* m);
void     LiveMapModel_LineFilterPush(LiveMapModel* m, int32_t x, int32_t y);
void     LiveMapModel_LineFilterPushEnd(LiveMapModel* m);

void     LiveMapModel_GetGPS_Info(LiveMapModel* m, GPS_Info_t* info);
void     LiveMapModel_GetArrowTheme(LiveMapModel* m, char* buf, uint32_t size);
bool     LiveMapModel_GetTrackFilterActive(LiveMapModel* m);
void     LiveMapModel_TrackReload(LiveMapModel* m, LiveMap_PointCb cb, void* user);
void     LiveMapModel_SetStatusBarStyle(LiveMapModel* m, StatusBar_Style_t style);

#ifdef __cplusplus
}
#endif

#endif
