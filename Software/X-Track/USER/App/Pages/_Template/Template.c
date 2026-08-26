/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING WITHOUT LIMITATION THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Template.h"
#include <string.h>

static TemplatePage s_template;

static void Template_OnEvent(lv_event_t* event);

/**
  * @brief  把当前 tick 和进入时保存的 tick 写到 Label
  */
static void Template_Update(TemplatePage* p)
{
    lv_label_set_text_fmt(
        p->view.ui.labelTick,
        "tick = %d save = %d",
        TemplateModel_GetData(),
        p->model.TickSave
    );
}

/**
  * @brief  给控件注册 onEvent
  * @param  obj  本模板挂的是页面 root
  */
static void Template_AttachEvent(TemplatePage* p, lv_obj_t* obj)
{
    lv_obj_set_user_data(obj, p);
    lv_obj_add_event_cb(obj, Template_OnEvent, LV_EVENT_ALL, p);
}

/**
  * @brief  LVGL 定时器回调，转调 Template_Update
  * @param  timer  user_data 为 TemplatePage*
  */
static void Template_OnTimerUpdate(lv_timer_t* timer)
{
    TemplatePage* p = (TemplatePage*)timer->user_data;
    Template_Update(p);
}

/**
  * @brief  事件入口：root 短按或 LEAVE 则 Pop
  */
static void Template_OnEvent(lv_event_t* event)
{
    TemplatePage* p = (TemplatePage*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(p);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == p->base._root)
    {
        if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LEAVE)
        {
            page_pop(&p->base);
        }
    }
}

static void on_custom_attr(PageBase* page)
{
    LV_LOG_USER("begin");
    Page_SetCustomCacheEnable(page, true);
    Page_SetCustomLoadAnimType(page, PAGE_LOAD_ANIM_OVER_BOTTOM, 1000, lv_anim_path_bounce);
}

static void on_load(PageBase* page)
{
    TemplatePage* p = (TemplatePage*)page;
    LV_LOG_USER("begin");
    TemplateView_Create(&p->view, page->_root);
    lv_label_set_text(p->view.ui.labelTitle, page->_Name);

    Template_AttachEvent(p, page->_root);

    p->model.TickSave = TemplateModel_GetData();
}

static void on_did_load(PageBase* page)
{
    (void)page;
    LV_LOG_USER("begin");
}

static void on_will_appear(PageBase* page)
{
    TemplatePage* p = (TemplatePage*)page;
    TemplateParam_t param;
    LV_LOG_USER("begin");

    param.color = lv_color_white();
    param.time = 1000;
    page_stash_pop(page, &param, sizeof(param));

    lv_obj_set_style_bg_color(page->_root, param.color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page->_root, LV_OPA_COVER, LV_PART_MAIN);
    p->timer = lv_timer_create(Template_OnTimerUpdate, param.time, p);
}

static void on_did_appear(PageBase* page)
{
    (void)page;
    LV_LOG_USER("begin");
}

static void on_will_disappear(PageBase* page)
{
    (void)page;
    LV_LOG_USER("begin");
}

static void on_did_disappear(PageBase* page)
{
    TemplatePage* p = (TemplatePage*)page;
    LV_LOG_USER("begin");
    lv_timer_del(p->timer);
}

static void on_unload(PageBase* page)
{
    (void)page;
    LV_LOG_USER("begin");
}

static void on_did_unload(PageBase* page)
{
    (void)page;
    LV_LOG_USER("begin");
}

static const PageOps s_template_ops = {
    .on_custom_attr    = on_custom_attr,
    .on_load           = on_load,
    .on_did_load       = on_did_load,
    .on_will_appear    = on_will_appear,
    .on_did_appear     = on_did_appear,
    .on_will_disappear = on_will_disappear,
    .on_did_disappear  = on_did_disappear,
    .on_unload         = on_unload,
    .on_did_unload     = on_did_unload,
    .destroy           = NULL,  /* 静态单例，不要 delete */
};

PageBase* Template_Create(void)
{
    memset(&s_template, 0, sizeof(s_template));
    s_template.base.ops = &s_template_ops;
    return &s_template.base;
}
