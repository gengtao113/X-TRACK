#ifndef __SYSTEM_INFOS_PRESENTER_H
#define __SYSTEM_INFOS_PRESENTER_H

#include "SystemInfosView.h"
#include "SystemInfosModel.h"

namespace Page
{

class SystemInfos
{
public:
    PageBase base;  /**< 必须是第一项，调度器只认 PageBase* */

    SystemInfos();
    ~SystemInfos();

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
    void Update();
    void AttachEvent(lv_obj_t* obj);
    static void onTimerUpdate(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    SystemInfosView View;
    SystemInfosModel Model;
    lv_timer_t* timer;
};

}

#endif
