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
#include "StartUp.h"
#include <string.h>

static StartupPage s_startup;

/**
  * @brief  一次性 timer：约 2 秒后换成表盘
  * @param  timer  user_data 为 StartupPage*
  */
static void Startup_OnTimer(lv_timer_t* timer)
{
    StartupPage* p = (StartupPage*)timer->user_data;
    page_replace(&p->base, "Pages/Dialplate", NULL);
}

static void on_custom_attr(PageBase* page)
{
    Page_SetCustomCacheEnable(page, false);
    Page_SetCustomLoadAnimType(page, PAGE_LOAD_ANIM_NONE, PAGE_ANIM_TIME_DEFAULT, PAGE_ANIM_PATH_DEFAULT);
}

static void on_load(PageBase* page)
{
    StartupPage* p = (StartupPage*)page;
    lv_timer_t* timer;

    StartupModel_Init(&p->model);
    StartupModel_SetEncoderEnable(false);
    StartupView_Create(&p->view, page->_root);

    timer = lv_timer_create(Startup_OnTimer, 2000, p);
    lv_timer_set_repeat_count(timer, 1);
}

static void on_did_load(PageBase* page)
{
    (void)page;
}

static void on_will_appear(PageBase* page)
{
    StartupPage* p = (StartupPage*)page;
    StartupModel_PlayMusic(&p->model, "Startup");
    lv_anim_timeline_start(p->view.ui.anim_timeline);
}

static void on_did_appear(PageBase* page)
{
    lv_obj_fade_out(page->_root, 500, 1500);
}

static void on_will_disappear(PageBase* page)
{
    (void)page;
}

static void on_did_disappear(PageBase* page)
{
    StartupPage* p = (StartupPage*)page;
    StartupModel_SetStatusBarAppear(&p->model, true);
}

static void on_unload(PageBase* page)
{
    StartupPage* p = (StartupPage*)page;
    StartupView_Delete(&p->view);
    StartupModel_SetEncoderEnable(true);
    StartupModel_Deinit(&p->model);
}

static void on_did_unload(PageBase* page)
{
    (void)page;
}

static const PageOps s_startup_ops = {
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

PageBase* Startup_Create(void)
{
    memset(&s_startup, 0, sizeof(s_startup));
    s_startup.base.ops = &s_startup_ops;
    return &s_startup.base;
}
