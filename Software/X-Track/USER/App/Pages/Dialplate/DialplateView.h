#ifndef __DIALPLATE_VIEW_H
#define __DIALPLATE_VIEW_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    lv_obj_t* cont;
    lv_obj_t* lableValue;
    lv_obj_t* lableUnit;
} DialplateSubInfo_t;

typedef struct
{
    struct
    {
        struct
        {
            lv_obj_t* cont;
            lv_obj_t* labelSpeed;
            lv_obj_t* labelUint;
        } topInfo;

        struct
        {
            lv_obj_t* cont;
            DialplateSubInfo_t labelInfoGrp[4];
        } bottomInfo;

        struct
        {
            lv_obj_t* cont;
            lv_obj_t* btnMap;
            lv_obj_t* btnRec;
            lv_obj_t* btnMenu;
        } btnCont;

        lv_anim_timeline_t* anim_timeline;
    } ui;
} DialplateView;

void DialplateView_Create(DialplateView* view, lv_obj_t* root);
void DialplateView_Delete(DialplateView* view);
void DialplateView_AppearAnimStart(DialplateView* view, bool reverse);

#ifdef __cplusplus
}
#endif

#endif
