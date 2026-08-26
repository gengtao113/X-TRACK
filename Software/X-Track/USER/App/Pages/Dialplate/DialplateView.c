#include "DialplateView.h"
#include "Resource/ResourcePool.h"
#include "Utils/lv_ext/lv_anim_timeline_wrapper.h"
#include "Utils/lv_ext/lv_obj_ext_func.h"

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

static void DialplateView_TopInfo_Create(DialplateView* view, lv_obj_t* par);
static void DialplateView_BottomInfo_Create(DialplateView* view, lv_obj_t* par);
static void DialplateView_SubInfoGrp_Create(lv_obj_t* par, DialplateSubInfo_t* info, const char* unitText);
static void DialplateView_BtnCont_Create(DialplateView* view, lv_obj_t* par);
static lv_obj_t* DialplateView_Btn_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs);

void DialplateView_Create(DialplateView* view, lv_obj_t* root)
{
    DialplateView_BottomInfo_Create(view, root);
    DialplateView_TopInfo_Create(view, root);
    DialplateView_BtnCont_Create(view, root);

    view->ui.anim_timeline = lv_anim_timeline_create();

#define ANIM_DEF(start_time, obj, attr, start, end) \
    {start_time, obj, LV_ANIM_EXEC(attr), start, end, 500, lv_anim_path_ease_out, true}

#define ANIM_OPA_DEF(start_time, obj) \
    ANIM_DEF(start_time, obj, opa_scale, LV_OPA_TRANSP, LV_OPA_COVER)

    lv_coord_t y_tar_top = lv_obj_get_y(view->ui.topInfo.cont);
    lv_coord_t y_tar_bottom = lv_obj_get_y(view->ui.bottomInfo.cont);
    lv_coord_t h_tar_btn = lv_obj_get_height(view->ui.btnCont.btnRec);

    lv_anim_timeline_wrapper_t wrapper[] =
    {
        ANIM_DEF(0, view->ui.topInfo.cont, y, -lv_obj_get_height(view->ui.topInfo.cont), y_tar_top),
        ANIM_DEF(200, view->ui.bottomInfo.cont, y, -lv_obj_get_height(view->ui.bottomInfo.cont), y_tar_bottom),
        ANIM_OPA_DEF(200, view->ui.bottomInfo.cont),
        ANIM_DEF(500, view->ui.btnCont.btnMap, height, 0, h_tar_btn),
        ANIM_DEF(600, view->ui.btnCont.btnRec, height, 0, h_tar_btn),
        ANIM_DEF(700, view->ui.btnCont.btnMenu, height, 0, h_tar_btn),
        LV_ANIM_TIMELINE_WRAPPER_END
    };
    lv_anim_timeline_add_wrapper(view->ui.anim_timeline, wrapper);
}

void DialplateView_Delete(DialplateView* view)
{
    if (view->ui.anim_timeline)
    {
        lv_anim_timeline_del(view->ui.anim_timeline);
        view->ui.anim_timeline = NULL;
    }
}

static void DialplateView_TopInfo_Create(DialplateView* view, lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 142);

    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);

    lv_obj_set_style_radius(cont, 27, 0);
    lv_obj_set_y(cont, -36);
    view->ui.topInfo.cont = cont;

    lv_obj_t* label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_65"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "00");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 63);
    view->ui.topInfo.labelSpeed = label;

    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_17"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "km/h");
    lv_obj_align_to(label, view->ui.topInfo.labelSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    view->ui.topInfo.labelUint = label;
}

static void DialplateView_BottomInfo_Create(DialplateView* view, lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    int i;
    const char* unitText[4] =
    {
        "AVG",
        "Time",
        "Trip",
        "Calorie"
    };

    lv_obj_remove_style_all(cont);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_size(cont, LV_HOR_RES, 90);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 106);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    view->ui.bottomInfo.cont = cont;

    for (i = 0; i < (int)ARRAY_SIZE(view->ui.bottomInfo.labelInfoGrp); i++)
    {
        DialplateView_SubInfoGrp_Create(
            cont,
            &(view->ui.bottomInfo.labelInfoGrp[i]),
            unitText[i]
        );
    }
}

static void DialplateView_SubInfoGrp_Create(lv_obj_t* par, DialplateSubInfo_t* info, const char* unitText)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_t* label;

    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 93, 39);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_AROUND,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_17"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    info->lableValue = label;

    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("bahnschrift_13"), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xb3b3b3), 0);
    lv_label_set_text(label, unitText);
    info->lableUnit = label;

    info->cont = cont;
}

static void DialplateView_BtnCont_Create(DialplateView* view, lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 40);
    lv_obj_align_to(cont, view->ui.bottomInfo.cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    view->ui.btnCont.cont = cont;

    view->ui.btnCont.btnMap = DialplateView_Btn_Create(cont, ResourcePool_GetImage("locate"), -80);
    view->ui.btnCont.btnRec = DialplateView_Btn_Create(cont, ResourcePool_GetImage("start"), 0);
    view->ui.btnCont.btnMenu = DialplateView_Btn_Create(cont, ResourcePool_GetImage("menu"), 80);
}

static lv_obj_t* DialplateView_Btn_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs)
{
    lv_obj_t* obj = lv_obj_create(par);
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] = { LV_STYLE_WIDTH, LV_STYLE_HEIGHT, LV_STYLE_PROP_INV };

    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 40, 31);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, 0);
    lv_obj_set_style_bg_img_src(obj, img_src, 0);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_width(obj, 45, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, 25, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x666666), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xbbbbbb), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff931e), LV_STATE_FOCUSED);
    lv_obj_set_style_radius(obj, 9, 0);

    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_ease_out,
        200,
        0,
        NULL
    );
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_FOCUSED);

    lv_obj_update_layout(obj);

    return obj;
}

void DialplateView_AppearAnimStart(DialplateView* view, bool reverse)
{
    lv_anim_timeline_set_reverse(view->ui.anim_timeline, reverse);
    lv_anim_timeline_start(view->ui.anim_timeline);
}
