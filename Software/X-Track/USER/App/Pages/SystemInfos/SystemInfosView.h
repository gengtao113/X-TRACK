#ifndef __SYSTEM_INFOS_VIEW_H
#define __SYSTEM_INFOS_VIEW_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    lv_obj_t* cont;
    lv_obj_t* icon;
    lv_obj_t* labelInfo;
    lv_obj_t* labelData;
} SystemInfosItem_t;

typedef struct
{
    struct
    {
        SystemInfosItem_t sport;
        SystemInfosItem_t gps;
        SystemInfosItem_t mag;
        SystemInfosItem_t imu;
        SystemInfosItem_t rtc;
        SystemInfosItem_t battery;
        SystemInfosItem_t storage;
        SystemInfosItem_t system;
    } ui;

    struct
    {
        lv_style_t icon;
        lv_style_t focus;
        lv_style_t info;
        lv_style_t data;
    } style;
} SystemInfosView;

void SystemInfosView_Create(SystemInfosView* view, lv_obj_t* root);
void SystemInfosView_Delete(SystemInfosView* view);
void SystemInfosView_SetSport(SystemInfosView* view, float trip, const char* time, float maxSpd);
void SystemInfosView_SetGPS(SystemInfosView* view, float lat, float lng, float alt, const char* utc, float course, float speed);
void SystemInfosView_SetMAG(SystemInfosView* view, float dir, int x, int y, int z);
void SystemInfosView_SetIMU(SystemInfosView* view, int step, const char* info);
void SystemInfosView_SetRTC(SystemInfosView* view, const char* dateTime);
void SystemInfosView_SetBattery(SystemInfosView* view, int usage, float voltage, const char* state);
void SystemInfosView_SetStorage(SystemInfosView* view, const char* detect, const char* size, const char* type, const char* version);
void SystemInfosView_SetSystem(SystemInfosView* view, const char* firmVer, const char* authorName, const char* lvglVer, const char* bootTime, const char* compilerName, const char* bulidTime);
void SystemInfosView_SetScrollToY(lv_obj_t* obj, lv_coord_t y, lv_anim_enable_t en);
void SystemInfosView_OnFocus(lv_group_t* g);

#ifdef __cplusplus
}
#endif

#endif
