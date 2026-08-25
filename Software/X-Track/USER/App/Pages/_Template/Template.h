#ifndef __TEMPLATE_PRESENTER_H
#define __TEMPLATE_PRESENTER_H

#include "TemplateView.h"
#include "TemplateModel.h"

namespace Page
{

/**
  * @brief  新页面脚手架 Presenter（产品路径不会 Push 本页）
  * @note   复制本目录改类名即可。示范：缓存、切页动画、stash 传参、timer、Pop。
  *         C 对应 struct { Page base; TemplateView view; TemplateModel model; timer; }。
  */
class Template : public PageBase
{
public:
    /**
      * @brief  Push 时经 stash 传入的参数
      * @note   onViewWillAppear 里 PAGE_STASH_POP 取出。没有 stash 则用默认白底、1s。
      */
    typedef struct
    {
        uint16_t time;     /**< 刷新周期，毫秒 */
        lv_color_t color;  /**< 页面背景色 */
    } Param_t;

public:
    /**
      * @brief  构造
      * @note   timer 置空，不建控件。
      */
    Template();

    /**
      * @brief  析构
      * @note   本模板析构体为空。
      */
    virtual ~Template();

    /**
      * @brief  自定义页面属性（Install 之后、LOAD 之前）
      * @note   开缓存；切页动画从底部弹入 1000ms，曲线 bounce。
      */
    virtual void onCustomAttrConfig();

    /**
      * @brief  页面开始加载
      * @note   View.Create(_root)，标题写成页面名 _Name，给 root 挂事件，记下当前 tick。
      */
    virtual void onViewLoad();

    /**
      * @brief  页面加载结束
      * @note   模板无额外工作。
      */
    virtual void onViewDidLoad();

    /**
      * @brief  页面即将显示
      * @note   取 stash（失败用默认），设背景色，按 param.time 创建刷新 timer。
      */
    virtual void onViewWillAppear();

    /**
      * @brief  页面已显示
      * @note   模板无额外工作。
      */
    virtual void onViewDidAppear();

    /**
      * @brief  页面即将离开
      * @note   模板无额外工作。
      */
    virtual void onViewWillDisappear();

    /**
      * @brief  页面已离开
      * @note   删除刷新 timer。
      */
    virtual void onViewDidDisappear();

    /**
      * @brief  页面卸载
      * @note   模板未调 View.Delete / Model.Deinit（空壳）。
      */
    virtual void onViewUnload();

    /**
      * @brief  页面卸载结束
      * @note   模板无额外工作。
      */
    virtual void onViewDidUnload();

private:
    /**
      * @brief  把当前 tick 和进入时保存的 tick 写到 Label
      */
    void Update();

    /**
      * @brief  给控件注册 onEvent
      * @param  obj  本模板挂的是页面 root
      * @note   同时 lv_obj_set_user_data(obj, this)。
      */
    void AttachEvent(lv_obj_t* obj);

    /**
      * @brief  LVGL 定时器回调，转调 Update()
      * @param  timer  user_data 为 Template*
      * @note   static，没有隐藏的 this。
      */
    static void onTimerUpdate(lv_timer_t* timer);

    /**
      * @brief  事件入口：root 短按或 LEAVE 则 Pop
      * @param  event  LVGL 事件
      */
    static void onEvent(lv_event_t* event);

private:
    TemplateView View;     /**< 标题 + tick 两个 Label */
    TemplateModel Model;   /**< 只提供 lv_tick_get() */
    lv_timer_t* timer;     /**< WillAppear 创建，DidDisappear 删除 */
};

}

#endif
