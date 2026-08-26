#ifndef __SYSTEM_INFOS_PRESENTER_H
#define __SYSTEM_INFOS_PRESENTER_H

#include "SystemInfosView.h"
#include "SystemInfosModel.h"
#include "Utils/PageManager/PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    PageBase base;
    SystemInfosView view;
    SystemInfosModel model;
    lv_timer_t* timer;
} SystemInfosPage;

PageBase* SystemInfos_Create(void);

#ifdef __cplusplus
}
#endif

#endif
