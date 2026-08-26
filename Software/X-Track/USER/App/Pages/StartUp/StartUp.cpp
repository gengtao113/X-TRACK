#include "StartUp.h"

using namespace Page;

Startup::Startup()
{
    base.ops = &PageOpsBinder<Startup>::ops;
}

Startup::~Startup()
{

}

void Startup::onCustomAttrConfig()
{
    Page_SetCustomCacheEnable(&base, false);
    Page_SetCustomLoadAnimType(&base, PageManager::LOAD_ANIM_NONE, PAGE_ANIM_TIME_DEFAULT, PAGE_ANIM_PATH_DEFAULT);
}

void Startup::onViewLoad()
{
    Model.Init();
    Model.SetEncoderEnable(false);
    View.Create(base._root);
    lv_timer_t* timer = lv_timer_create(onTimer, 2000, this);
    lv_timer_set_repeat_count(timer, 1);
}

void Startup::onViewDidLoad()
{
}

void Startup::onViewWillAppear()
{
    Model.PlayMusic("Startup");
    lv_anim_timeline_start(View.ui.anim_timeline);
}

void Startup::onViewDidAppear()
{
    lv_obj_fade_out(base._root, 500, 1500);
}

void Startup::onViewWillDisappear()
{

}

void Startup::onViewDidDisappear()
{
    Model.SetStatusBarAppear(true);
}

void Startup::onViewUnload()
{
    View.Delete();
    Model.SetEncoderEnable(true);
    Model.Deinit();
}

void Startup::onViewDidUnload()
{
}

void Startup::onTimer(lv_timer_t* timer)
{
    Startup* instance = (Startup*)timer->user_data;

    instance->base._Manager->Replace("Pages/Dialplate");
}

void Startup::onEvent(lv_event_t* event)
{
    Startup* instance = (Startup*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->base._root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            //instance->Manager->Pop();
        }
    }
}
