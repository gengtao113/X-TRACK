#include "Dialplate.h"

using namespace Page;

/**
  * @brief  构造表盘 Presenter
  * @note   C 里相当于 Dialplate_Create()：只初始化成员，不建控件。
  *         recState 默认 READY；lastFocus 为空，首次焦点落在录轨键。
  * @retval None
  */
Dialplate::Dialplate()
    : recState(RECORD_STATE_READY)
    , lastFocus(nullptr)
{
    base.ops = &PageOpsBinder<Dialplate>::ops;
}

/**
  * @brief  析构表盘 Presenter
  * @note   控件和 Model 在 onViewUnload 里已经释放，这里为空。
  * @retval None
  */
Dialplate::~Dialplate()
{
}

/**
  * @brief  自定义页面属性（Install 之后、LOAD 之前由 PageManager 调用）
  * @note   关掉切页滑场动画，开机 Replace 到表盘时不要整页滑动。
  * @retval None
  */
void Dialplate::onCustomAttrConfig()
{
    Page_SetCustomLoadAnimType(&base, PageManager::LOAD_ANIM_NONE, PAGE_ANIM_TIME_DEFAULT, PAGE_ANIM_PATH_DEFAULT);
}

/**
  * @brief  页面开始加载：创建 Model / View，并给三个按钮挂事件
  * @note   PageManager 此时已建好 base._root。C 对应 on_load：
  *         Model.Init()、View.Create(root)、lv_obj_add_event_cb。
  * @retval None
  */
void Dialplate::onViewLoad()
{
    Model.Init();
    View.Create(base._root);

    AttachEvent(View.ui.btnCont.btnMap);
    AttachEvent(View.ui.btnCont.btnRec);
    AttachEvent(View.ui.btnCont.btnMenu);
}

/**
  * @brief  页面加载结束
  * @note   表盘无额外工作。
  * @retval None
  */
void Dialplate::onViewDidLoad()
{

}

/**
  * @brief  页面即将显示：挂编码器焦点、刷一帧、播入场动画
  * @note   切页时可能还按着键，先 wait_release 吃掉，避免误触发。
  *         group 不循环换焦；优先恢复 lastFocus，否则聚焦录轨键。
  * @retval None
  */
void Dialplate::onViewWillAppear()
{
    lv_indev_wait_release(lv_indev_get_act());
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);

    lv_group_set_wrap(group, false);

    lv_group_add_obj(group, View.ui.btnCont.btnMap);
    lv_group_add_obj(group, View.ui.btnCont.btnRec);
    lv_group_add_obj(group, View.ui.btnCont.btnMenu);

    if (lastFocus)
    {
        lv_group_focus_obj(lastFocus);
    }
    else
    {
        lv_group_focus_obj(View.ui.btnCont.btnRec);
    }

    Model.SetStatusBarStyle(DataProc::STATUS_BAR_STYLE_TRANSP);

    Update();

    View.AppearAnimStart();
}

/**
  * @brief  页面已显示：启动 1 秒刷新定时器
  * @note   必须在 DidAppear 才开 timer；WillDisappear 里成对删除。
  *         user_data 传入 this，C 里就是把 DialplatePage* 塞进 timer。
  * @retval None
  */
void Dialplate::onViewDidAppear()
{
    timer = lv_timer_create(onTimerUpdate, 1000, this);
}

/**
  * @brief  页面即将离开：记住焦点、摘掉 group、删除刷新定时器
  * @note   先停 timer 再切页，避免回调改已经隐藏/删除的控件。
  * @retval None
  */
void Dialplate::onViewWillDisappear()
{
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);
    lastFocus = lv_group_get_focused(group);
    lv_group_remove_all_objs(group);
    lv_timer_del(timer);
    //View.AppearAnimStart(true);
}

/**
  * @brief  页面已离开
  * @note   表盘无额外工作；root 若走缓存则只是隐藏。
  * @retval None
  */
void Dialplate::onViewDidDisappear()
{
}

/**
  * @brief  页面卸载：释放 Model 与 View
  * @note   C 对应 on_unload。base._root 由 PageManager 异步删除，这里只删动画时间线。
  * @retval None
  */
void Dialplate::onViewUnload()
{
    Model.Deinit();
    View.Delete();
}

/**
  * @brief  页面卸载结束
  * @note   表盘无额外工作。
  * @retval None
  */
void Dialplate::onViewDidUnload()
{

}

/**
  * @brief  给控件注册事件回调
  * @param  obj: 目标 lv_obj（地图 / 录轨 / 菜单按钮）
  * @note   user_data 传 this。static onEvent 里再转回 Dialplate*。
  * @retval None
  */
void Dialplate::AttachEvent(lv_obj_t* obj)
{
    /* obj          : 要监听的按钮
     * onEvent      : static 回调（没有 this，必须用下面的 user_data 找回对象）
     * LV_EVENT_ALL : 短按、长按等全部事件都进 onEvent，在里面再按 code 分支
     * this         : 作为 user_data 传入，onEvent 里 lv_event_get_user_data 取出 */
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

/**
  * @brief  从 Model 取数，刷新表盘上的速度 / 均速 / 时间 / 里程 / 卡路里
  * @note   只改 Label 文字，不读传感器。singleDistance 单位是米，显示时 /1000 换成 km。
  * @retval None
  */
void Dialplate::Update()
{
    char buf[16];  ///< MakeTimeString 的输出缓冲

    /* 上半大号时速，整数，不足两位补 0；GetSpeed 读 sportStatusInfo.speedKph */
    lv_label_set_text_fmt(View.ui.topInfo.labelSpeed, "%02d", (int)Model.GetSpeed());

    lv_label_set_text_fmt(View.ui.bottomInfo.labelInfoGrp[0].lableValue, "%0.1f km/h", Model.GetAvgSpeed());

    /* [1] Time：单次骑行秒数 -> "HH:MM:SS"，写入 buf 再设到 Label */
    lv_label_set_text(
        View.ui.bottomInfo.labelInfoGrp[1].lableValue,
        DataProc::MakeTimeString(Model.sportStatusInfo.singleTime, buf, sizeof(buf))
    );

    /* [2] Trip：单次里程，内部是米，/1000 显示成 km */
    lv_label_set_text_fmt(
        View.ui.bottomInfo.labelInfoGrp[2].lableValue,
        "%0.1f km",
        Model.sportStatusInfo.singleDistance / 1000
    );

    /* [3] Calorie：单次热量，整数，单位显示为 k */
    lv_label_set_text_fmt(
        View.ui.bottomInfo.labelInfoGrp[3].lableValue,
        "%d k",
        int(Model.sportStatusInfo.singleCalorie)
    );
}

/**
  * @brief  LVGL 定时器回调：每秒刷新一次表盘数字
  * @param  timer: LVGL 定时器，user_data 为 Dialplate*
  * @note   static 成员 = C 的普通函数，没有隐藏的 this，必须从 user_data 取回对象。
  * @retval None
  */
void Dialplate::onTimerUpdate(lv_timer_t* timer)
{
    Dialplate* instance = (Dialplate*)timer->user_data;

    instance->Update();
}

/**
  * @brief  处理地图键、菜单键的短按（页面跳转）
  * @param  btn: 被点击的按钮对象
  * @note   地图 → Push LiveMap；菜单 → Push SystemInfos。录轨键不在这里处理。
  * @retval None
  */
void Dialplate::onBtnClicked(lv_obj_t* btn)
{
    if (btn == View.ui.btnCont.btnMap)           ///< 地图键短按
    {
        base._Manager->Push("Pages/LiveMap");         ///< 压栈进入地图；名字是 Install 的 appName
    }
    else if (btn == View.ui.btnCont.btnMenu)     ///< 菜单键短按
    {
        base._Manager->Push("Pages/SystemInfos");     ///< 压栈进入系统信息；返回时对方 Pop
    }
    /* 录轨键也会进本函数，两个 if 都对不上，直接返回。录轨在 onEvent 里另调 onRecord。 */
}

/**
  * @brief  录轨按钮状态机（界面状态，真正开文件在 Model.RecorderCommand）
  * @param  longPress: true = 长按，false = 短按
  * @note   READY --长按且 GPS 就绪--> RUN
  *         RUN   --短按--> PAUSE
  *         PAUSE --短按--> RUN；--长按--> STOP（预备结束）
  *         STOP  --短按--> RUN；--长按--> READY（真正停止）
  * @retval None
  */
void Dialplate::onRecord(bool longPress)
{
    switch (recState)
    {
    case RECORD_STATE_READY:
        if (longPress)
        {
            if (!Model.GetGPSReady())
            {
                LV_LOG_WARN("GPS has not ready, can't start record");
                Model.PlayMusic("Error");
                return;
            }

            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_START);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    case RECORD_STATE_RUN:
        if (!longPress)
        {
            Model.PlayMusic("UnstableConnect");
            Model.RecorderCommand(Model.REC_PAUSE);
            SetBtnRecImgSrc("start");
            recState = RECORD_STATE_PAUSE;
        }
        break;
    case RECORD_STATE_PAUSE:
        if (longPress)
        {
            Model.PlayMusic("NoOperationWarning");
            SetBtnRecImgSrc("stop");
            Model.RecorderCommand(Model.REC_READY_STOP);
            recState = RECORD_STATE_STOP;
        }
        else
        {
            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_CONTINUE);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    case RECORD_STATE_STOP:
        if (longPress)
        {
            Model.PlayMusic("Disconnect");
            Model.RecorderCommand(Model.REC_STOP);
            SetBtnRecImgSrc("start");
            recState = RECORD_STATE_READY;
        }
        else
        {
            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_CONTINUE);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    default:
        break;
    }
}

/**
  * @brief  更换录轨按钮背景图（start / pause / stop）
  * @param  srcName: 资源池中的图片名
  * @retval None
  */
void Dialplate::SetBtnRecImgSrc(const char* srcName)
{
    lv_obj_set_style_bg_img_src(View.ui.btnCont.btnRec, ResourcePool::GetImage(srcName), 0);
}

/**
  * @brief  三个按钮的 LVGL 事件总入口
  * @param  event: LVGL 事件
  * @note   static 回调。短按先走 onBtnClicked（地图/菜单跳转）；
  *         若目标是录轨键，短按/长按再进 onRecord。
  *         因此录轨键短按会先被 onBtnClicked 看到，但 if 对不上地图/菜单，无副作用。
  * @retval None
  */
void Dialplate::onEvent(lv_event_t* event)
{
    Dialplate* instance = (Dialplate*)lv_event_get_user_data(event);  ///< static 无 this，从 AttachEvent 传入的 user_data 还原对象
    LV_ASSERT_NULL(instance);                                        ///< 空指针则断言（漏传 this 时尽早发现）

    lv_obj_t* obj = lv_event_get_current_target(event);              ///< 触发本事件的控件（地图 / 录轨 / 菜单之一）
    lv_event_code_t code = lv_event_get_code(event);                 ///< 事件类型：短按、长按等

    if (code == LV_EVENT_SHORT_CLICKED)                              ///< 任意按钮短按
    {
        instance->onBtnClicked(obj);                                 ///< 地图/菜单在此 Push 页面；录轨键对不上分支
    }

    if (obj == instance->View.ui.btnCont.btnRec)                     ///< 仅录轨键再走录轨状态机
    {
        if (code == LV_EVENT_SHORT_CLICKED)                          ///< 短按：暂停或继续
        {
            instance->onRecord(false);                               ///< longPress = false
        }
        else if (code == LV_EVENT_LONG_PRESSED)                      ///< 长按：开始 / 预备停止 / 真正停止
        {
            instance->onRecord(true);                                ///< longPress = true
        }
    }
}
