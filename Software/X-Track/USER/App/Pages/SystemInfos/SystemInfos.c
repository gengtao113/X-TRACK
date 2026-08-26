#include "SystemInfos.h"
#include "Common/DataProc/dataproc_c.h"
#include "Version.h"
#include <string.h>

static SystemInfosPage s_system_infos;

static void SystemInfos_OnEvent(lv_event_t* event);
static void SystemInfos_Update(SystemInfosPage* p);

static void SystemInfos_AttachEvent(SystemInfosPage* p, lv_obj_t* obj)
{
    lv_obj_add_event_cb(obj, SystemInfos_OnEvent, LV_EVENT_ALL, p);
}

static void SystemInfos_Update(SystemInfosPage* p)
{
    char buf[64];
    float trip;
    float maxSpd;
    float lat;
    float lng;
    float alt;
    float course;
    float speed;
    float dir;
    int x;
    int y;
    int z;
    int steps;
    int usage;
    float voltage;
    bool detect;
    const char* type = "-";

    SystemInfosModel_GetSportInfo(&p->model, &trip, buf, sizeof(buf), &maxSpd);
    SystemInfosView_SetSport(&p->view, trip, buf, maxSpd);

    SystemInfosModel_GetGPSInfo(&p->model, &lat, &lng, &alt, buf, sizeof(buf), &course, &speed);
    SystemInfosView_SetGPS(&p->view, lat, lng, alt, buf, course, speed);

    SystemInfosModel_GetMAGInfo(&p->model, &dir, &x, &y, &z);
    SystemInfosView_SetMAG(&p->view, dir, x, y, z);

    SystemInfosModel_GetIMUInfo(&p->model, &steps, buf, sizeof(buf));
    SystemInfosView_SetIMU(&p->view, steps, buf);

    SystemInfosModel_GetRTCInfo(&p->model, buf, sizeof(buf));
    SystemInfosView_SetRTC(&p->view, buf);

    SystemInfosModel_GetBatteryInfo(&p->model, &usage, &voltage, buf, sizeof(buf));
    SystemInfosView_SetBattery(&p->view, usage, voltage, buf);

    SystemInfosModel_GetStorageInfo(&p->model, &detect, &type, buf, sizeof(buf));
    SystemInfosView_SetStorage(
        &p->view,
        detect ? "OK" : "ERROR",
        buf,
        type,
        VERSION_FILESYSTEM
    );

    DataProc_MakeTimeString(lv_tick_get(), buf, sizeof(buf));
    SystemInfosView_SetSystem(
        &p->view,
        VERSION_FIRMWARE_NAME " " VERSION_SOFTWARE,
        VERSION_AUTHOR_NAME,
        VERSION_LVGL,
        buf,
        VERSION_COMPILER,
        VERSION_BUILD_TIME
    );
}

static void SystemInfos_OnTimerUpdate(lv_timer_t* timer)
{
    SystemInfosPage* p = (SystemInfosPage*)timer->user_data;
    SystemInfos_Update(p);
}

static void SystemInfos_OnEvent(lv_event_t* event)
{
    SystemInfosPage* p = (SystemInfosPage*)lv_event_get_user_data(event);
    lv_obj_t* obj;
    lv_event_code_t code;

    LV_ASSERT_NULL(p);

    obj = lv_event_get_current_target(event);
    code = lv_event_get_code(event);

    if (code == LV_EVENT_PRESSED)
    {
        if (lv_obj_has_state(obj, LV_STATE_FOCUSED))
        {
            page_pop(&p->base);
        }
    }

    if (obj == p->base._root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            page_pop(&p->base);
        }
    }
}

static void on_custom_attr(PageBase* page)
{
    (void)page;
}

static void on_load(PageBase* page)
{
    SystemInfosPage* p = (SystemInfosPage*)page;
    SystemInfosItem_t* item_grp;
    int i;

    SystemInfosModel_Init(&p->model);
    SystemInfosView_Create(&p->view, page->_root);
    SystemInfos_AttachEvent(p, page->_root);

    item_grp = (SystemInfosItem_t*)&p->view.ui;
    for (i = 0; i < (int)(sizeof(p->view.ui) / sizeof(SystemInfosItem_t)); i++)
    {
        SystemInfos_AttachEvent(p, item_grp[i].icon);
    }
}

static void on_did_load(PageBase* page)
{
    (void)page;
}

static void on_will_appear(PageBase* page)
{
    SystemInfosPage* p = (SystemInfosPage*)page;

    SystemInfosModel_SetStatusBarStyle(&p->model, STATUS_BAR_STYLE_BLACK);

    p->timer = lv_timer_create(SystemInfos_OnTimerUpdate, 1000, p);
    lv_timer_ready(p->timer);

    SystemInfosView_SetScrollToY(page->_root, -LV_VER_RES, LV_ANIM_OFF);
    lv_obj_set_style_opa(page->_root, LV_OPA_TRANSP, 0);
    lv_obj_fade_in(page->_root, 300, 0);
}

static void on_did_appear(PageBase* page)
{
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);
    SystemInfosView_OnFocus(group);
}

static void on_will_disappear(PageBase* page)
{
    lv_obj_fade_out(page->_root, 300, 0);
}

static void on_did_disappear(PageBase* page)
{
    SystemInfosPage* p = (SystemInfosPage*)page;
    lv_timer_del(p->timer);
}

static void on_unload(PageBase* page)
{
    SystemInfosPage* p = (SystemInfosPage*)page;
    SystemInfosView_Delete(&p->view);
    SystemInfosModel_Deinit(&p->model);
}

static void on_did_unload(PageBase* page)
{
    (void)page;
}

static const PageOps s_system_infos_ops = {
    .on_custom_attr    = on_custom_attr,
    .on_load           = on_load,
    .on_did_load       = on_did_load,
    .on_will_appear    = on_will_appear,
    .on_did_appear     = on_did_appear,
    .on_will_disappear = on_will_disappear,
    .on_did_disappear  = on_did_disappear,
    .on_unload         = on_unload,
    .on_did_unload     = on_did_unload,
    .destroy           = NULL,
};

PageBase* SystemInfos_Create(void)
{
    memset(&s_system_infos, 0, sizeof(s_system_infos));
    s_system_infos.base.ops = &s_system_infos_ops;
    return &s_system_infos.base;
}
