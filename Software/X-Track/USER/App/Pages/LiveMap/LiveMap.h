#ifndef __LIVEMAP_PRESENTER_H
#define __LIVEMAP_PRESENTER_H

#include "LiveMapView.h"
#include "LiveMapModel.h"
#include "Utils/PageManager/PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t lastMapUpdateTime;
    uint32_t lastContShowTime;
    lv_timer_t* timer;
    LiveMapPoint_t lastTileContOriPoint;
    bool isTrackAvtive;
} LiveMapPriv;

typedef struct
{
    PageBase base;
    LiveMapView view;
    LiveMapModel model;
    LiveMapPriv run;
} LiveMapPage;

PageBase* LiveMap_Create(void);

#ifdef __cplusplus
}
#endif

#endif
