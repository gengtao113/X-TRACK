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
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __PAGE_MANAGER_H
#define __PAGE_MANAGER_H

#include "PageBase.h"
#include "PageFactory.h"
#include <vector>
#include <stack>

/**
  * @brief  页面调度器：页面池 + 导航栈 + 生命周期状态机 + 切页动画
  * @note   App.cpp 里 static 一份。页面通过 PageBase::_Manager 拿到它再 Push/Pop。
  *         C 对应 struct { Page *pool[]; Page *stack[]; factory; current; prev; }。
  */
class PageManager
{
public:

    /**
      * @brief  切页动画类型
      * @note   OVER = 新页盖住旧页（旧页不动）；MOVE = 新页把旧页推走；GLOBAL = 跟全局设置。
      */
    typedef enum
    {
        LOAD_ANIM_GLOBAL = 0,   /**< 使用 SetGlobalLoadAnimType 的配置 */

        LOAD_ANIM_OVER_LEFT,    /**< 新页从右往左盖住 */
        LOAD_ANIM_OVER_RIGHT,
        LOAD_ANIM_OVER_TOP,     /**< 本工程 App.cpp 全局默认 */
        LOAD_ANIM_OVER_BOTTOM,

        LOAD_ANIM_MOVE_LEFT,    /**< 新页把旧页一起推走 */
        LOAD_ANIM_MOVE_RIGHT,
        LOAD_ANIM_MOVE_TOP,
        LOAD_ANIM_MOVE_BOTTOM,

        LOAD_ANIM_FADE_ON,      /**< 新页淡入、旧页淡出 */

        LOAD_ANIM_NONE,         /**< 无动画（表盘、开机页） */

        _LOAD_ANIM_LAST = LOAD_ANIM_NONE
    } LoadAnim_t;

    /**
      * @brief  根节点允许拖拽的方向（OVER 动画才开拖拽返回）
      */
    typedef enum
    {
        ROOT_DRAG_DIR_NONE,  /**< 不允许拖 */
        ROOT_DRAG_DIR_HOR,   /**< 水平拖 */
        ROOT_DRAG_DIR_VER,   /**< 垂直拖 */
    } RootDragDir_t;

    /** LVGL 动画写属性：如 lv_obj_set_x / set_y / set_style_opa */
    typedef void(*lv_anim_setter_t)(void*, int32_t);

    /** LVGL 动画读属性：取当前 x/y/opa 作为起点 */
    typedef int32_t(*lv_anim_getter_t)(void*);

    /**
      * @brief  进入方/离开方各自动画的起止值
      */
    typedef struct
    {
        struct
        {
            int32_t start;  /**< 进入方起点（如屏幕外） */
            int32_t end;    /**< 进入方终点（通常 0） */
        } enter;

        struct
        {
            int32_t start;
            int32_t end;    /**< 离开方终点（如滑出屏幕） */
        } exit;
    } AnimValue_t;

    /**
      * @brief  某一种 LoadAnim_t 对应的完整动画参数（见 PM_Anim.cpp 表）
      */
    typedef struct
    {
        lv_anim_setter_t setter;  /**< 改哪个属性 */
        lv_anim_getter_t getter;  /**< 读当前值 */
        RootDragDir_t dragDir;    /**< 可否拖拽返回 */
        AnimValue_t push;         /**< Push/Replace 进入时用 */
        AnimValue_t pop;          /**< Pop 返回时用（方向相反） */
    } LoadAnimAttr_t;

public:
    /**
      * @brief  构造调度器
      * @param  factory  按类名 new 页面，本工程是 AppFactory；可为空则不能 Install
      */
    PageManager(PageFactory* factory = nullptr);

    /**
      * @brief  析构：清空页面栈
      */
    ~PageManager();

    /**
      * @brief  按类名创建页面并登记到页面池
      * @param  className  工厂类名，如 "Dialplate"
      * @param  appName    Push 用的名字，如 "Pages/Dialplate"
      * @note   此时就 new 出对象，并调 onCustomAttrConfig。Push 不会再 new。
      * @retval 成功 true
      */
    bool Install(const char* className, const char* appName);

    /**
      * @brief  从池中卸掉并 delete 页面对象
      * @param  appName  Install 时的应用名
      * @retval 成功 true；仍在栈中则失败
      */
    bool Uninstall(const char* appName);

    /**
      * @brief  把已有 PageBase* 登记进池（Install 内部会调）
      * @param  base  页面对象
      * @param  name  应用名，写入 base->_Name，并设置 base->_Manager = this
      */
    bool Register(PageBase* base, const char* name);

    /**
      * @brief  从池中去掉登记，不 delete
      */
    bool Unregister(const char* name);

    /**
      * @brief  换成新页：栈顶出栈并强制卸 UI，新页入栈
      * @param  name   目标应用名
      * @param  stash  可选参数，传给新页
      * @note   开机 Startup → Dialplate 用这个，避免开机页压在栈底。
      */
    bool Replace(const char* name, const PageStash_t* stash = nullptr);

    /**
      * @brief  压栈进入新页，旧页仍在栈里（默认可缓存）
      * @param  name   目标应用名，必须已 Install，且不能已在栈中
      * @param  stash  可选参数
      */
    bool Push(const char* name, const PageStash_t* stash = nullptr);

    /**
      * @brief  弹出当前页，回到栈里下一页
      * @note   只剩根页时失败。被弹出页若自动缓存，会关掉缓存并 UNLOAD。
      */
    bool Pop();

    /**
      * @brief  清到栈底那一页（主界面）
      */
    bool BackHome();

    /**
      * @brief  上一页的名字（_PagePrev）
      * @retval 无则 "EMPTY_PAGE"
      */
    const char* GetPagePrevName();

    /**
      * @brief  设置全局默认切页动画（单页可用 SetCustomLoadAnimType 覆盖）
      * @param  anim  类型，默认 OVER_LEFT；本工程 App.cpp 设为 OVER_TOP
      * @param  time  时长 ms
      * @param  path  缓动曲线
      */
    void SetGlobalLoadAnimType(
        LoadAnim_t anim = LOAD_ANIM_OVER_LEFT,
        uint16_t time = 500,
        lv_anim_path_cb_t path = lv_anim_path_ease_out
    );

    /**
      * @brief  每个页面 LOAD 时给 _root 套上的默认样式（本工程是全屏黑底）
      */
    void SetRootDefaultStyle(lv_style_t* style)
    {
        _RootDefaultStyle = style;
    }

private:
    /**
      * @brief  按应用名在页面池里找对象
      */
    PageBase* FindPageInPool(const char* name);

    /**
      * @brief  按应用名看是否已在导航栈里（禁止同一页 Push 两次）
      */
    PageBase* FindPageInStack(const char* name);

    /** 栈顶页面，即当前应显示的页 */
    PageBase* GetStackTop();

    /** 栈顶下面那一页（底下被盖住的页） */
    PageBase* GetStackTopAfter();

    /**
      * @brief  清空栈并强制卸生命周期
      * @param  keepBottom  true = 留下栈底（BackHome 用）
      */
    void SetStackClear(bool keepBottom = false);

    /**
      * @brief  无动画强制走完离开并 UNLOAD（源码函数名拼写为 Fource）
      */
    bool FourceUnload(PageBase* base);

    /**
      * @brief  查动画参数表，填 LoadAnimAttr_t
      */
    bool GetLoadAnimAttr(uint8_t anim, LoadAnimAttr_t* attr);

    /** 是否 OVER 类动画（旧页不动，可拖拽） */
    bool GetIsOverAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_OVER_LEFT && anim <= LOAD_ANIM_OVER_BOTTOM);
    }

    /** 是否 MOVE 类动画（新旧一起动） */
    bool GetIsMoveAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_MOVE_LEFT && anim <= LOAD_ANIM_MOVE_BOTTOM);
    }

    /** 按全局/当前配置初始化一条 lv_anim_t */
    void AnimDefaultInit(lv_anim_t* a);

    /** 取当前这次切换正在用的动画参数 */
    bool GetCurrentLoadAnimAttr(LoadAnimAttr_t* attr)
    {
        return GetLoadAnimAttr(GetCurrentLoadAnimType(), attr);
    }

    /** 当前这次切换的动画类型 */
    LoadAnim_t GetCurrentLoadAnimType()
    {
        return (LoadAnim_t)_AnimState.Current.Type;
    }

    /** 根节点拖拽事件（手指拖当前页露出底下页） */
    static void onRootDragEvent(lv_event_t* event);
    static void onRootDragAnimFinish(lv_anim_t* a);
    /** 拖过阈值后异步 Pop */
    static void onRootAsyncLeave(void* base);
    void RootEnableDrag(lv_obj_t* root);
    static void RootGetDragPredict(lv_coord_t* x, lv_coord_t* y);

    /**
      * @brief  真正切换：拷 stash、改状态机、调图层
      * @param  base        目标页
      * @param  isEnterAct  true = Push/Replace 进入；false = Pop 返回
      * @param  stash       可选参数
      */
    bool SwitchTo(PageBase* base, bool isEnterAct, const PageStash_t* stash = nullptr);

    /** 切页动画结束：再 StateUpdate 进 DID_APPEAR / DID_DISAPPEAR */
    static void onSwitchAnimFinish(lv_anim_t* a);
    void SwitchAnimCreate(PageBase* base);
    /** 按目标页自定义动画或全局动画，更新 _AnimState.Current */
    void SwitchAnimTypeUpdate(PageBase* base);
    /** 双方动画都结束才清 IsSwitchReq */
    bool SwitchReqCheck();
    /** 动画忙则拒绝新的 Push/Pop */
    bool SwitchAnimStateCheck();

    PageState_t StateLoadExecute(PageBase* base);
    PageState_t StateWillAppearExecute(PageBase* base);
    PageState_t StateDidAppearExecute(PageBase* base);
    PageState_t StateWillDisappearExecute(PageBase* base);
    PageState_t StateDidDisappearExecute(PageBase* base);
    PageState_t StateUnloadExecute(PageBase* base);

    /**
      * @brief  按 base->priv.State 跑一步状态机并回调 onViewXxx
      */
    void StateUpdate(PageBase* base);

    /** 当前页的生命周期状态 */
    PageState_t GetState()
    {
        return _PageCurrent->priv.State;
    }

private:

    PageFactory* _Factory;           /**< 按类名 new 页面，本工程 AppFactory */

    std::vector<PageBase*> _PagePool; /**< 页面池：已 Install 的对象，C 就是指针数组 */

    std::stack<PageBase*> _PageStack; /**< 导航栈：栈顶 = 正在显示的页 */

    PageBase* _PagePrev;              /**< 上一页（刚切走的那页） */
    PageBase* _PageCurrent;           /**< 当前页（正在进入或已在前台） */

    /**
      * @brief  切页动画总状态（一次切换共用）
      */
    struct
    {
        bool IsSwitchReq;              /**< 已发出切换请求，尚未全部完成 */
        bool IsBusy;                   /**< 正在切 */
        bool IsEntering;               /**< true = 进入动作；false = Pop 退出 */

        PageAnimAttr_t Current;  /**< 本次实际使用的动画 */
        PageAnimAttr_t Global;   /**< SetGlobalLoadAnimType 保存的全局值 */
    } _AnimState;

    lv_style_t* _RootDefaultStyle;    /**< LOAD 时加到每个 _root 上 */
};

#endif
