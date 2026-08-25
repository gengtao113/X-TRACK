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
#ifndef __PAGE_BASE_H
#define __PAGE_BASE_H

#include "lvgl/lvgl.h"

/**
  * @brief  把变量打包成 Stash_t，供 Push/Replace 第二个参数使用
  * @note   例：_Manager->Push("Pages/Xxx", PAGE_STASH_MAKE(param));
  */
#define PAGE_STASH_MAKE(data) {&(data), sizeof(data)}

/**
  * @brief  从本页 stash 取出数据到变量（宏展开为 this->StashPop）
  * @note   须与 PAGE_STASH_MAKE 的类型/大小一致。失败时变量保持原值。
  */
#define PAGE_STASH_POP(data)  this->StashPop(&(data), sizeof(data))

#define PAGE_ANIM_TIME_DEFAULT 500 /**< 默认切页动画时长，毫秒 */

#define PAGE_ANIM_PATH_DEFAULT lv_anim_path_ease_out /**< 默认切页动画曲线 */

class PageManager;  /**< 前置声明：指针成员用，避免头文件互相 include */

/**
  * @brief  所有页面的公共基类（生命周期回调 + 调度器指针）
  * @note   C 对应「结构体公共头 + 函数指针表」。Dialplate 等 inherit 后填自己的 onViewXxx。
  *         PageManager 只认 PageBase*，不关心具体是表盘还是地图。
  */
class PageBase
{
public:

    /**
      * @brief  页面生命周期状态，由 PageManager::StateUpdate 驱动
      * @note   有缓存时 DID_DISAPPEAR 后回到 WILL_APPEAR，不走 UNLOAD。
      */
    typedef enum
    {
        PAGE_STATE_IDLE,            /**< 未加载，_root 为空 */
        PAGE_STATE_LOAD,            /**< 创建 root，调 onViewLoad / onViewDidLoad */
        PAGE_STATE_WILL_APPEAR,     /**< 即将显示，调 onViewWillAppear，开切页动画 */
        PAGE_STATE_DID_APPEAR,      /**< 动画结束，调 onViewDidAppear */
        PAGE_STATE_ACTIVITY,        /**< 前台，用户正在看 */
        PAGE_STATE_WILL_DISAPPEAR,  /**< 即将离开，调 onViewWillDisappear */
        PAGE_STATE_DID_DISAPPEAR,   /**< 已隐藏，调 onViewDidDisappear */
        PAGE_STATE_UNLOAD,          /**< 卸 UI，调 onViewUnload / onViewDidUnload */
        _PAGE_STATE_LAST
    } State_t;

    /**
      * @brief  切页时附带的一块参数
      * @note   SwitchTo 里 memcpy 到目标页 priv.Stash，目标页 PAGE_STASH_POP 取走。
      */
    typedef struct
    {
        void* ptr;       /**< 数据指针 */
        uint32_t size;   /**< 字节数，Pop 时必须匹配 */
    } Stash_t;

    /**
      * @brief  本页切页动画配置
      * @note   Type 为 PageManager::LoadAnim_t；LOAD_ANIM_GLOBAL 则跟全局设置。
      */
    typedef struct
    {
        uint8_t Type;              /**< 动画类型 */
        uint16_t Time;             /**< 时长 ms */
        lv_anim_path_cb_t Path;    /**< 缓动曲线 */
    } AnimAttr_t;

public:
    lv_obj_t* _root;       /**< 本页 LVGL 根对象；未 LOAD 或已 UNLOAD 时为 NULL */
    PageManager* _Manager; /**< 指向 App.cpp 里那个唯一的 PageManager，用来 Push/Pop */
    const char* _Name;     /**< 应用名，如 "Pages/Dialplate"（Install 第二个参数） */
    uint16_t _ID;          /**< 页面 ID，本工程几乎未用 */
    void* _UserData;       /**< 用户指针，本工程几乎未用 */

    /**
      * @brief  调度器私有状态（约定只给 PageManager 改，页面不要直接写）
      */
    struct
    {
        bool ReqEnableCache;        /**< 手动缓存：是否保留 root */
        bool ReqDisableAutoCache;   /**< true = 关掉自动缓存，改用 ReqEnableCache */

        bool IsDisableAutoCache;    /**< 本次进入时同步下来的「是否手动缓存」 */
        bool IsCached;              /**< 当前 root 是否缓存（切走后还在） */

        Stash_t Stash;              /**< Push 带过来的参数区 */
        State_t State;              /**< 生命周期状态 */

        /** 切页动画运行时状态 */
        struct
        {
            bool IsEnter;           /**< true = 本页是进入方；false = 离开方 */
            bool IsBusy;            /**< 本页动画是否还在播 */
            AnimAttr_t Attr;        /**< 本页自定义动画；Type=GLOBAL 则用全局 */
        } Anim;
    } priv;

public:
    /**
      * @brief  虚析构，保证 delete PageBase* 时调用子类析构
      */
    virtual ~PageBase() {}

    /**
      * @brief  自定义页面属性（Install 之后立刻调用，尚未 LOAD）
      * @note   适合 SetCustomCacheEnable / SetCustomLoadAnimType。默认空实现。
      */
    virtual void onCustomAttrConfig() {}

    /**
      * @brief  页面开始加载：创建控件、申请资源
      * @note   此时 _root 已由调度器创建。对应 C 的 on_load。
      */
    virtual void onViewLoad() {}

    /**
      * @brief  页面加载结束
      */
    virtual void onViewDidLoad() {}

    /**
      * @brief  即将显示：挂输入、刷一帧、播入场动画
      */
    virtual void onViewWillAppear() {}

    /**
      * @brief  已经显示：开刷新 timer 等
      */
    virtual void onViewDidAppear() {}

    /**
      * @brief  即将离开：停 timer、摘编码器 group
      */
    virtual void onViewWillDisappear() {}

    /**
      * @brief  已经离开（root 已隐藏）
      */
    virtual void onViewDidDisappear() {}

    /**
      * @brief  开始卸载：释放 Model / View；_root 由调度器异步删除
      */
    virtual void onViewUnload() {}

    /**
      * @brief  卸载结束，状态回到 IDLE
      */
    virtual void onViewDidUnload() {}

    /**
      * @brief  手动指定是否缓存页面（同时关掉自动缓存）
      * @param  en  true = 切走保留 root；false = 切走 UNLOAD
      */
    void SetCustomCacheEnable(bool en);

    /**
      * @brief  是否使用自动缓存
      * @param  en  true = 调度器自动决定（默认进页后缓存）；false = 改用手动
      */
    void SetCustomAutoCacheEnable(bool en);

    /**
      * @brief  设置本页切页动画
      * @param  animType  PageManager::LoadAnim_t，或 LOAD_ANIM_GLOBAL 跟全局
      * @param  time      时长 ms，默认 500
      * @param  path      缓动曲线，默认 ease_out
      */
    void SetCustomLoadAnimType(
        uint8_t animType,
        uint16_t time = PAGE_ANIM_TIME_DEFAULT,
        lv_anim_path_cb_t path = PAGE_ANIM_PATH_DEFAULT
    );

    /**
      * @brief  从 stash 拷出数据并释放内部缓冲
      * @param  ptr   目标缓冲区
      * @param  size  必须等于 stash 的 size
      * @retval true 成功；无 stash 或长度不匹配则 false
      */
    bool StashPop(void* ptr, uint32_t size);
};

#endif // ! __PAGE_BASE_H
