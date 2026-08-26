#ifndef __STARTUP_PRESENTER_H
#define __STARTUP_PRESENTER_H

#include "StartUpView.h"
#include "StartUpModel.h"

namespace Page
{

class Startup
{
public:
    PageBase base;  /**< 必须是第一项，调度器只认 PageBase* */

    Startup();
    ~Startup();

    void onCustomAttrConfig();
    void onViewLoad();
    void onViewDidLoad();
    void onViewWillAppear();
    void onViewDidAppear();
    void onViewWillDisappear();
    void onViewDidDisappear();
    void onViewUnload();
    void onViewDidUnload();

private:
    static void onTimer(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    StartupView View;
    StartupModel Model;
};

}

#endif
