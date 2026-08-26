#ifndef __DIALPLATE_PRESENTER_H
#define __DIALPLATE_PRESENTER_H

#include "DialplateView.h"
#include "DialplateModel.h"

namespace Page
{

/**
  * @brief  表盘 Presenter：粘合 View 与 Model，处理生命周期和按键
  * @note   C 对应 struct { Page base; DialplateView view; DialplateModel model; ... }。
  *         继承 PageBase，由 PageManager 按状态机调用下面的 onViewXxx。
  *         不读 GPS 寄存器，不 lv_obj_create 整页布局。
  */
class Dialplate
{
public:
    PageBase base;  /**< 必须是第一项，调度器只认 PageBase* */

    /**
      * @brief  构造表盘 Presenter
      * @note   只初始化成员，不建控件。recState = READY，lastFocus = NULL。
      */
    Dialplate();

    /**
      * @brief  析构表盘 Presenter
      * @note   控件和 Model 在 onViewUnload 里释放，析构体为空。
      */
    ~Dialplate();

    /**
      * @brief  自定义页面属性（Install 之后、LOAD 之前调用）
      * @note   本页关掉切页滑场动画（LOAD_ANIM_NONE）。
      */
    void onCustomAttrConfig();

    /**
      * @brief  页面开始加载
      * @note   Model.Init()、View.Create(_root)，并给三个按钮挂事件。
      */
    void onViewLoad();

    /**
      * @brief  页面加载结束
      * @note   表盘无额外工作。
      */
    void onViewDidLoad();

    /**
      * @brief  页面即将显示
      * @note   挂编码器 group、恢复焦点、刷一帧、播入场动画。
      */
    void onViewWillAppear();

    /**
      * @brief  页面已显示
      * @note   创建 1 秒刷新定时器，user_data 为 this。
      */
    void onViewDidAppear();

    /**
      * @brief  页面即将离开
      * @note   记住焦点、从 group 摘掉按钮、删除刷新定时器。
      */
    void onViewWillDisappear();

    /**
      * @brief  页面已离开
      * @note   表盘无额外工作。
      */
    void onViewDidDisappear();

    /**
      * @brief  页面卸载
      * @note   Model.Deinit()、View.Delete()。_root 由 PageManager 删除。
      */
    void onViewUnload();

    /**
      * @brief  页面卸载结束
      * @note   表盘无额外工作。
      */
    void onViewDidUnload();

private:
    /**
      * @brief  录轨界面状态（不是 Recorder 节点内部状态）
      * @note   READY 长按开始 → RUN 短按暂停 → PAUSE 长按预备结束 → STOP 长按真正停止。
      */
    typedef enum
    {
        RECORD_STATE_READY,  /**< 未录制，等待长按开始 */
        RECORD_STATE_RUN,    /**< 正在录制 */
        RECORD_STATE_PAUSE,  /**< 已暂停 */
        RECORD_STATE_STOP    /**< 预备停止，长按才 REC_STOP */
    } RecordState_t;

private:
    /**
      * @brief  从 Model 取数，刷新速度 / 均速 / 时间 / 里程 / 卡路里
      */
    void Update();

    /**
      * @brief  给控件注册 onEvent 回调
      * @param  obj  目标按钮（地图 / 录轨 / 菜单）
      * @note   user_data 传 this。
      */
    void AttachEvent(lv_obj_t* obj);

    /**
      * @brief  LVGL 定时器回调，每秒调用 Update()
      * @param  timer  user_data 为 Dialplate*
      * @note   static = C 普通函数，没有隐藏的 this。
      */
    static void onTimerUpdate(lv_timer_t* timer);

    /**
      * @brief  三个按钮的 LVGL 事件总入口
      * @param  event  LVGL 事件
      * @note   短按走 onBtnClicked；录轨键短按/长按再走 onRecord。
      */
    static void onEvent(lv_event_t* event);

    /**
      * @brief  处理地图键、菜单键短按（页面跳转）
      * @param  btn  被点击的按钮
      */
    void onBtnClicked(lv_obj_t* btn);

    /**
      * @brief  录轨按钮状态机
      * @param  longPress  true = 长按，false = 短按
      * @note   真正开/停文件走 Model.RecorderCommand()。
      */
    void onRecord(bool longPress);

    /**
      * @brief  更换录轨按钮背景图
      * @param  srcName  资源池图片名（start / pause / stop）
      */
    void SetBtnRecImgSrc(const char* srcName);

private:
    DialplateView View;       /**< 控件句柄，只碰 LVGL */
    DialplateModel Model;     /**< 数据与命令，只碰 DataCenter */
    lv_timer_t* timer;        /**< 1s 刷数定时器，DidAppear 创建、WillDisappear 删除 */
    RecordState_t recState;   /**< 录轨 UI 状态 */
    lv_obj_t* lastFocus;      /**< 离开页时记住的编码器焦点 */
};

}

#endif
