#include "LiveMapView.h"
#include <stdbool.h>
#include "Config/Config.h"
#include "Resource/ResourcePool.h"
#include <stdio.h>

#if CONFIG_MAP_IMG_PNG_ENABLE
#include "Utils/lv_img_png/lv_img_png.h"
#  define TILE_IMG_CREATE  lv_img_png_create
#  define TILE_IMG_SET_SRC lv_img_png_set_src
#else
#  define TILE_IMG_CREATE  lv_img_create
#  define TILE_IMG_SET_SRC lv_img_set_src
#endif

static void LiveMapView_Style_Create(LiveMapView* view);
static void LiveMapView_Map_Create(LiveMapView* view, lv_obj_t* par, uint32_t tileNum);
static void LiveMapView_ZoomCtrl_Create(LiveMapView* view, lv_obj_t* par);
static void LiveMapView_SportInfo_Create(LiveMapView* view, lv_obj_t* par);
static lv_obj_t* LiveMapView_ImgLabel_Create(LiveMapView* view, lv_obj_t* par, const void* img_src, lv_coord_t x_ofs, lv_coord_t y_ofs);
static void LiveMapView_Track_Create(LiveMapView* view, lv_obj_t* par);

void LiveMapView_Create(LiveMapView* view, lv_obj_t* root, uint32_t tileNum)
{
    lv_obj_t* label;

    lv_obj_set_style_bg_color(root, lv_color_white(), 0);

    label = lv_label_create(root);
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_17"), 0);
    lv_label_set_text(label, "LOADING...");
    view->ui.labelInfo = label;

    LiveMapView_Style_Create(view);
    LiveMapView_Map_Create(view, root, tileNum);
    LiveMapView_ZoomCtrl_Create(view, root);
    LiveMapView_SportInfo_Create(view, root);
}

void LiveMapView_Delete(LiveMapView* view)
{
    if (view->ui.track.lineTrack)
    {
        lv_poly_line_destroy(view->ui.track.lineTrack);
        view->ui.track.lineTrack = NULL;
    }

    if (view->ui.map.imgTiles)
    {
        lv_mem_free(view->ui.map.imgTiles);
        view->ui.map.imgTiles = NULL;
    }

    lv_style_reset(&view->ui.styleCont);
    lv_style_reset(&view->ui.styleLabel);
    lv_style_reset(&view->ui.styleLine);
}

static void LiveMapView_Style_Create(LiveMapView* view)
{
    lv_style_init(&view->ui.styleCont);
    lv_style_set_bg_color(&view->ui.styleCont, lv_color_black());
    lv_style_set_bg_opa(&view->ui.styleCont, LV_OPA_60);
    lv_style_set_radius(&view->ui.styleCont, 6);
    lv_style_set_shadow_width(&view->ui.styleCont, 10);
    lv_style_set_shadow_color(&view->ui.styleCont, lv_color_black());

    lv_style_init(&view->ui.styleLabel);
    lv_style_set_text_font(&view->ui.styleLabel, ResourcePool_GetFont("bahnschrift_17"));
    lv_style_set_text_color(&view->ui.styleLabel, lv_color_white());

    lv_style_init(&view->ui.styleLine);
    lv_style_set_line_color(&view->ui.styleLine, lv_color_hex(0xff931e));
    lv_style_set_line_width(&view->ui.styleLine, 5);
    lv_style_set_line_opa(&view->ui.styleLine, LV_OPA_COVER);
    lv_style_set_line_rounded(&view->ui.styleLine, true);
}

static void LiveMapView_Map_Create(LiveMapView* view, lv_obj_t* par, uint32_t tileNum)
{
    lv_obj_t* cont;
    lv_obj_t* img;
    lv_img_t* imgOri;
    uint32_t i;

    cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    lv_obj_set_style_outline_color(cont, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_outline_width(cont, 2, 0);
#endif
    view->ui.map.cont = cont;

    view->ui.map.imgTiles = (lv_obj_t**)lv_mem_alloc(tileNum * sizeof(lv_obj_t*));
    view->ui.map.tileNum = tileNum;

    for (i = 0; i < tileNum; i++)
    {
        img = TILE_IMG_CREATE(cont);
        lv_obj_remove_style_all(img);
        view->ui.map.imgTiles[i] = img;
    }

    LiveMapView_Track_Create(view, cont);

    img = lv_img_create(cont);
    lv_img_set_src(img, ResourcePool_GetImage("gps_arrow_dark"));

    imgOri = (lv_img_t*)img;
    lv_obj_set_pos(img, -imgOri->w, -imgOri->h);
    view->ui.map.imgArrow = img;
}

void LiveMapView_SetMapTile(LiveMapView* view, uint32_t tileSize, uint32_t widthCnt)
{
    uint32_t tileNum = view->ui.map.tileNum;
    lv_coord_t width = (lv_coord_t)(tileSize * widthCnt);
    lv_coord_t height = (lv_coord_t)(tileSize * (view->ui.map.tileNum / widthCnt));
    uint32_t i;

    lv_obj_set_size(view->ui.map.cont, width, height);

    for (i = 0; i < tileNum; i++)
    {
        lv_obj_t* img = view->ui.map.imgTiles[i];
        lv_coord_t x = (lv_coord_t)((i % widthCnt) * tileSize);
        lv_coord_t y = (lv_coord_t)((i / widthCnt) * tileSize);

        lv_obj_set_size(img, (lv_coord_t)tileSize, (lv_coord_t)tileSize);
        lv_obj_set_pos(img, x, y);
    }
}

void LiveMapView_SetMapTileSrc(LiveMapView* view, uint32_t index, const char* src)
{
    if (index >= view->ui.map.tileNum)
    {
        return;
    }

    TILE_IMG_SET_SRC(view->ui.map.imgTiles[index], src);
}

void LiveMapView_SetArrowTheme(LiveMapView* view, const char* theme)
{
    char buf[32];
    const void* src;

    snprintf(buf, sizeof(buf), "gps_arrow_%s", theme);
    src = ResourcePool_GetImage(buf);

    if (src == NULL)
    {
        ResourcePool_GetImage("gps_arrow_default");
    }

    lv_img_set_src(view->ui.map.imgArrow, src);
}

void LiveMapView_SetImgArrowStatus(LiveMapView* view, lv_coord_t x, lv_coord_t y, float angle)
{
    lv_obj_t* img = view->ui.map.imgArrow;
    lv_obj_set_pos(img, x, y);
    lv_img_set_angle(img, (int16_t)(angle * 10));
}

void LiveMapView_SetLineActivePoint(LiveMapView* view, lv_coord_t x, lv_coord_t y)
{
    lv_point_t end_point;
    if (!lv_poly_line_get_end_point(view->ui.track.lineTrack, &end_point))
    {
        return;
    }

    view->ui.track.pointActive[0] = end_point;
    view->ui.track.pointActive[1].x = x;
    view->ui.track.pointActive[1].y = y;
    lv_line_set_points(view->ui.track.lineActive, view->ui.track.pointActive, 2);
}

static void LiveMapView_ZoomCtrl_Create(LiveMapView* view, lv_obj_t* par)
{
    static const lv_style_prop_t prop[] =
    {
        LV_STYLE_X,
        LV_STYLE_OPA,
        LV_STYLE_PROP_INV
    };
    static lv_style_transition_dsc_t tran;
    lv_obj_t* cont;
    lv_obj_t* label;
    lv_obj_t* slider;

    cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_add_style(cont, &view->ui.styleCont, 0);
    lv_obj_set_style_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_size(cont, 50, 30);
    lv_obj_set_pos(cont, lv_obj_get_style_width(par, 0) - lv_obj_get_style_width(cont, 0) + 5, 40);
    view->ui.zoom.cont = cont;

    lv_style_transition_dsc_init(&tran, prop, lv_anim_path_ease_out, 200, 0, NULL);
    lv_obj_set_style_x(cont, lv_obj_get_style_width(par, 0), LV_STATE_USER_1);
    lv_obj_set_style_opa(cont, LV_OPA_TRANSP, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, LV_STATE_DEFAULT);
    lv_obj_add_state(cont, LV_STATE_USER_1);

    label = lv_label_create(cont);
    lv_obj_add_style(label, &view->ui.styleLabel, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, -2, 0);
    lv_label_set_text(label, "--");
    view->ui.zoom.labelInfo = label;

    slider = lv_slider_create(cont);
    lv_obj_remove_style_all(slider);
    lv_slider_set_value(slider, 15, LV_ANIM_OFF);
    view->ui.zoom.slider = slider;
}

static void LiveMapView_SportInfo_Create(LiveMapView* view, lv_obj_t* par)
{
    lv_obj_t* obj;
    lv_obj_t* label;

    obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_add_style(obj, &view->ui.styleCont, 0);
    lv_obj_set_size(obj, 159, 66);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, -10, 10);
    lv_obj_set_style_radius(obj, 10, 0);
    view->ui.sportInfo.cont = obj;

    label = lv_label_create(obj);
    lv_label_set_text(label, "00");
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_32"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, -10);
    view->ui.sportInfo.labelSpeed = label;

    label = lv_label_create(obj);
    lv_label_set_text(label, "km/h");
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_13"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align_to(label, view->ui.sportInfo.labelSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    view->ui.sportInfo.labelTrip = LiveMapView_ImgLabel_Create(view, obj, ResourcePool_GetImage("trip"), 5, 10);
    view->ui.sportInfo.labelTime = LiveMapView_ImgLabel_Create(view, obj, ResourcePool_GetImage("alarm"), 5, 30);
}

static lv_obj_t* LiveMapView_ImgLabel_Create(LiveMapView* view, lv_obj_t* par, const void* img_src, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    lv_obj_t* img = lv_img_create(par);
    lv_obj_t* label;

    lv_img_set_src(img, img_src);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, y_ofs);

    label = lv_label_create(par);
    lv_label_set_text(label, "--");
    lv_obj_add_style(label, &view->ui.styleLabel, 0);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, x_ofs, 0);
    return label;
}

static void LiveMapView_Track_Create(LiveMapView* view, lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_t* line;

    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    view->ui.track.cont = cont;

    view->ui.track.lineTrack = lv_poly_line_create(cont);
    lv_poly_line_set_style(view->ui.track.lineTrack, &view->ui.styleLine);

    line = lv_line_create(cont);
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, &view->ui.styleLine, 0);
#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    lv_obj_set_style_line_color(line, lv_palette_main(LV_PALETTE_BLUE), 0);
#endif
    view->ui.track.lineActive = line;
}
