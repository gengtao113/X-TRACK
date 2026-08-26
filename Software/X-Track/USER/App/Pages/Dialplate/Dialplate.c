#include "Dialplate.h"
#include "Common/DataProc/dataproc_c.h"
#include "Resource/ResourcePool.h"
#include <string.h>

static DialplatePage s_dialplate;

static void Dialplate_OnEvent(lv_event_t* event);
static void Dialplate_Update(DialplatePage* d);
static void Dialplate_AttachEvent(DialplatePage* d, lv_obj_t* obj);
static void Dialplate_OnBtnClicked(DialplatePage* d, lv_obj_t* btn);
static void Dialplate_OnRecord(DialplatePage* d, bool longPress);
static void Dialplate_SetBtnRecImgSrc(DialplatePage* d, const char* srcName);

static void Dialplate_AttachEvent(DialplatePage* d, lv_obj_t* obj)
{
    lv_obj_add_event_cb(obj, Dialplate_OnEvent, LV_EVENT_ALL, d);
}

static void Dialplate_Update(DialplatePage* d)
{
    char buf[16];

    lv_label_set_text_fmt(d->view.ui.topInfo.labelSpeed, "%02d", (int)DialplateModel_GetSpeed(&d->model));
    lv_label_set_text_fmt(d->view.ui.bottomInfo.labelInfoGrp[0].lableValue, "%0.1f km/h", DialplateModel_GetAvgSpeed(&d->model));

    lv_label_set_text(
        d->view.ui.bottomInfo.labelInfoGrp[1].lableValue,
        DataProc_MakeTimeString(d->model.sportStatusInfo.singleTime, buf, sizeof(buf))
    );

    lv_label_set_text_fmt(
        d->view.ui.bottomInfo.labelInfoGrp[2].lableValue,
        "%0.1f km",
        d->model.sportStatusInfo.singleDistance / 1000
    );

    lv_label_set_text_fmt(
        d->view.ui.bottomInfo.labelInfoGrp[3].lableValue,
        "%d k",
        (int)d->model.sportStatusInfo.singleCalorie
    );
}

static void Dialplate_OnTimerUpdate(lv_timer_t* timer)
{
    DialplatePage* d = (DialplatePage*)timer->user_data;
    Dialplate_Update(d);
}

static void Dialplate_OnBtnClicked(DialplatePage* d, lv_obj_t* btn)
{
    if (btn == d->view.ui.btnCont.btnMap)
    {
        page_push(&d->base, "Pages/LiveMap", NULL);
    }
    else if (btn == d->view.ui.btnCont.btnMenu)
    {
        page_push(&d->base, "Pages/SystemInfos", NULL);
    }
}

static void Dialplate_SetBtnRecImgSrc(DialplatePage* d, const char* srcName)
{
    lv_obj_set_style_bg_img_src(d->view.ui.btnCont.btnRec, ResourcePool_GetImage(srcName), 0);
}

static void Dialplate_OnRecord(DialplatePage* d, bool longPress)
{
    switch (d->rec_state)
    {
    case DIALPLATE_RECORD_STATE_READY:
        if (longPress)
        {
            if (!DialplateModel_GetGPSReady(&d->model))
            {
                LV_LOG_WARN("GPS has not ready, can't start record");
                DialplateModel_PlayMusic(&d->model, "Error");
                return;
            }

            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_START);
            Dialplate_SetBtnRecImgSrc(d, "pause");
            d->rec_state = DIALPLATE_RECORD_STATE_RUN;
        }
        break;
    case DIALPLATE_RECORD_STATE_RUN:
        if (!longPress)
        {
            DialplateModel_PlayMusic(&d->model, "UnstableConnect");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_PAUSE);
            Dialplate_SetBtnRecImgSrc(d, "start");
            d->rec_state = DIALPLATE_RECORD_STATE_PAUSE;
        }
        break;
    case DIALPLATE_RECORD_STATE_PAUSE:
        if (longPress)
        {
            DialplateModel_PlayMusic(&d->model, "NoOperationWarning");
            Dialplate_SetBtnRecImgSrc(d, "stop");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_READY_STOP);
            d->rec_state = DIALPLATE_RECORD_STATE_STOP;
        }
        else
        {
            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_CONTINUE);
            Dialplate_SetBtnRecImgSrc(d, "pause");
            d->rec_state = DIALPLATE_RECORD_STATE_RUN;
        }
        break;
    case DIALPLATE_RECORD_STATE_STOP:
        if (longPress)
        {
            DialplateModel_PlayMusic(&d->model, "Disconnect");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_STOP);
            Dialplate_SetBtnRecImgSrc(d, "start");
            d->rec_state = DIALPLATE_RECORD_STATE_READY;
        }
        else
        {
            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCommand(&d->model, DIALPLATE_REC_CONTINUE);
            Dialplate_SetBtnRecImgSrc(d, "pause");
            d->rec_state = DIALPLATE_RECORD_STATE_RUN;
        }
        break;
    default:
        break;
    }
}

static void Dialplate_OnEvent(lv_event_t* event)
{
    DialplatePage* d = (DialplatePage*)lv_event_get_user_data(event);
    lv_obj_t* obj;
    lv_event_code_t code;

    LV_ASSERT_NULL(d);

    obj = lv_event_get_current_target(event);
    code = lv_event_get_code(event);

    if (code == LV_EVENT_SHORT_CLICKED)
    {
        Dialplate_OnBtnClicked(d, obj);
    }

    if (obj == d->view.ui.btnCont.btnRec)
    {
        if (code == LV_EVENT_SHORT_CLICKED)
        {
            Dialplate_OnRecord(d, false);
        }
        else if (code == LV_EVENT_LONG_PRESSED)
        {
            Dialplate_OnRecord(d, true);
        }
    }
}

static void on_custom_attr(PageBase* page)
{
    Page_SetCustomLoadAnimType(page, PAGE_LOAD_ANIM_NONE, PAGE_ANIM_TIME_DEFAULT, PAGE_ANIM_PATH_DEFAULT);
}

static void on_load(PageBase* page)
{
    DialplatePage* d = (DialplatePage*)page;
    DialplateModel_Init(&d->model);
    DialplateView_Create(&d->view, page->_root);

    Dialplate_AttachEvent(d, d->view.ui.btnCont.btnMap);
    Dialplate_AttachEvent(d, d->view.ui.btnCont.btnRec);
    Dialplate_AttachEvent(d, d->view.ui.btnCont.btnMenu);
}

static void on_did_load(PageBase* page)
{
    (void)page;
}

static void on_will_appear(PageBase* page)
{
    DialplatePage* d = (DialplatePage*)page;
    lv_group_t* group;

    lv_indev_wait_release(lv_indev_get_act());
    group = lv_group_get_default();
    LV_ASSERT_NULL(group);

    lv_group_set_wrap(group, false);

    lv_group_add_obj(group, d->view.ui.btnCont.btnMap);
    lv_group_add_obj(group, d->view.ui.btnCont.btnRec);
    lv_group_add_obj(group, d->view.ui.btnCont.btnMenu);

    if (d->last_focus)
    {
        lv_group_focus_obj(d->last_focus);
    }
    else
    {
        lv_group_focus_obj(d->view.ui.btnCont.btnRec);
    }

    DialplateModel_SetStatusBarStyle(&d->model, STATUS_BAR_STYLE_TRANSP);
    Dialplate_Update(d);
    DialplateView_AppearAnimStart(&d->view, false);
}

static void on_did_appear(PageBase* page)
{
    DialplatePage* d = (DialplatePage*)page;
    d->timer = lv_timer_create(Dialplate_OnTimerUpdate, 1000, d);
}

static void on_will_disappear(PageBase* page)
{
    DialplatePage* d = (DialplatePage*)page;
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);
    d->last_focus = lv_group_get_focused(group);
    lv_group_remove_all_objs(group);
    lv_timer_del(d->timer);
}

static void on_did_disappear(PageBase* page)
{
    (void)page;
}

static void on_unload(PageBase* page)
{
    DialplatePage* d = (DialplatePage*)page;
    DialplateModel_Deinit(&d->model);
    DialplateView_Delete(&d->view);
}

static void on_did_unload(PageBase* page)
{
    (void)page;
}

static const PageOps s_dialplate_ops = {
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

PageBase* Dialplate_Create(void)
{
    memset(&s_dialplate, 0, sizeof(s_dialplate));
    s_dialplate.rec_state = DIALPLATE_RECORD_STATE_READY;
    s_dialplate.last_focus = NULL;
    s_dialplate.base.ops = &s_dialplate_ops;
    return &s_dialplate.base;
}
