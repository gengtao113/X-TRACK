#ifndef __LIVEMAP_VIEW_H
#define __LIVEMAP_VIEW_H

#include "lvgl/lvgl.h"
#include "Utils/lv_poly_line/lv_poly_line_c.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    struct
    {
        lv_obj_t* labelInfo;

        lv_style_t styleCont;
        lv_style_t styleLabel;
        lv_style_t styleLine;

        struct
        {
            lv_obj_t* cont;
            lv_obj_t* imgArrow;
            lv_obj_t** imgTiles;
            uint32_t tileNum;
        } map;

        struct
        {
            lv_obj_t* cont;
            lv_poly_line* lineTrack;
            lv_obj_t* lineActive;
            lv_point_t pointActive[2];
        } track;

        struct
        {
            lv_obj_t* cont;
            lv_obj_t* labelInfo;
            lv_obj_t* slider;
        } zoom;

        struct
        {
            lv_obj_t* cont;
        } move;

        struct
        {
            lv_obj_t* cont;
            lv_obj_t* labelSpeed;
            lv_obj_t* labelTrip;
            lv_obj_t* labelTime;
        } sportInfo;
    } ui;
} LiveMapView;

void LiveMapView_Create(LiveMapView* view, lv_obj_t* root, uint32_t tileNum);
void LiveMapView_Delete(LiveMapView* view);
void LiveMapView_SetImgArrowStatus(LiveMapView* view, lv_coord_t x, lv_coord_t y, float angle);
void LiveMapView_SetMapTile(LiveMapView* view, uint32_t tileSize, uint32_t widthCnt);
void LiveMapView_SetMapTileSrc(LiveMapView* view, uint32_t index, const char* src);
void LiveMapView_SetArrowTheme(LiveMapView* view, const char* theme);
void LiveMapView_SetLineActivePoint(LiveMapView* view, lv_coord_t x, lv_coord_t y);

#ifdef __cplusplus
}
#endif

#endif
