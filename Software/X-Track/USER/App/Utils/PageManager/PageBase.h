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
#ifndef __PAGE_BASE_H
#define __PAGE_BASE_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct PageManager PageManager;

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  把变量打包成 Stash_t，供 Push/Replace 第二个参数使用
  * @note   例：base._Manager->Push("Pages/Xxx", PAGE_STASH_MAKE(param));
  */
#define PAGE_STASH_MAKE(data) {&(data), sizeof(data)}

/**
  * @brief  从本页 stash 取出数据到变量
  * @note   C++ 页面成员函数里用 PAGE_STASH_POP(param)，依赖成员名 base。
  *         C 里用 page_stash_pop(page, &param, sizeof(param))。
  */
#ifdef __cplusplus
#define PAGE_STASH_POP(data)  page_stash_pop(&base, &(data), sizeof(data))
#endif

#define PAGE_ANIM_TIME_DEFAULT 500 /**< 默认切页动画时长，毫秒 */

#define PAGE_ANIM_PATH_DEFAULT lv_anim_path_ease_out /**< 默认切页动画曲线 */

/**
  * @brief  切页动画类型（给 C 页面用）
  * @note   数值必须与 PageManager::LoadAnim_t 一致。C++ 页仍可用 PageManager::LOAD_ANIM_*。
  */
typedef enum
{
    PAGE_LOAD_ANIM_GLOBAL = 0,   /**< 跟全局 SetGlobalLoadAnimType */
    PAGE_LOAD_ANIM_OVER_LEFT = 1,
    PAGE_LOAD_ANIM_OVER_RIGHT = 2,
    PAGE_LOAD_ANIM_OVER_TOP = 3,
    PAGE_LOAD_ANIM_OVER_BOTTOM = 4, /**< 模板页：从底部弹入 */
    PAGE_LOAD_ANIM_MOVE_LEFT = 5,
    PAGE_LOAD_ANIM_MOVE_RIGHT = 6,
    PAGE_LOAD_ANIM_MOVE_TOP = 7,
    PAGE_LOAD_ANIM_MOVE_BOTTOM = 8,
    PAGE_LOAD_ANIM_FADE_ON = 9,
    PAGE_LOAD_ANIM_NONE = 10
} PageLoadAnim_t;

/**
  * @brief  页面生命周期状态，由 PageManager::StateUpdate 驱动
  * @note   有缓存时 DID_DISAPPEAR 后回到 WILL_APPEAR，不走 UNLOAD。
  */
typedef enum
{
    PAGE_STATE_IDLE,            /**< 未加载，_root 为空 */
    PAGE_STATE_LOAD,            /**< 创建 root，调 on_load / on_did_load */
    PAGE_STATE_WILL_APPEAR,     /**< 即将显示，调 on_will_appear，开切页动画 */
    PAGE_STATE_DID_APPEAR,      /**< 动画结束，调 on_did_appear */
    PAGE_STATE_ACTIVITY,        /**< 前台，用户正在看 */
    PAGE_STATE_WILL_DISAPPEAR,  /**< 即将离开，调 on_will_disappear */
    PAGE_STATE_DID_DISAPPEAR,   /**< 已隐藏，调 on_did_disappear */
    PAGE_STATE_UNLOAD,          /**< 卸 UI，调 on_unload / on_did_unload */
    _PAGE_STATE_LAST
} PageState_t;

/**
  * @brief  切页时附带的一块参数
  * @note   SwitchTo 里 memcpy 到目标页 priv.Stash，目标页 page_stash_pop 取走。
  */
typedef struct
{
    void* ptr;       /**< 数据指针 */
    uint32_t size;   /**< 字节数，Pop 时必须匹配 */
} PageStash_t;

/**
  * @brief  本页切页动画配置
  * @note   Type 为 PageManager::LoadAnim_t；LOAD_ANIM_GLOBAL 则跟全局设置。
  */
typedef struct
{
    uint8_t Type;              /**< 动画类型 */
    uint16_t Time;             /**< 时长 ms */
    lv_anim_path_cb_t Path;    /**< 缓动曲线 */
} PageAnimAttr_t;

typedef struct PageBase PageBase;

/**
  * @brief  页面生命周期函数指针表（替代 C++ virtual）
  * @note   空回调可填 NULL。destroy 供 Uninstall 释放 new 出来的对象。
  */
typedef struct
{
    void (*on_custom_attr)(PageBase *p);
    void (*on_load)(PageBase *p);
    void (*on_did_load)(PageBase *p);
    void (*on_will_appear)(PageBase *p);
    void (*on_did_appear)(PageBase *p);
    void (*on_will_disappear)(PageBase *p);
    void (*on_did_disappear)(PageBase *p);
    void (*on_unload)(PageBase *p);
    void (*on_did_unload)(PageBase *p);
    void (*destroy)(PageBase *p);
} PageOps;

/**
  * @brief  所有页面的公共头（生命周期回调 + 调度器指针）
  * @note   具体页面把 PageBase 放在结构体第一项。PageManager 只认 PageBase*。
  */
struct PageBase
{
    lv_obj_t* _root;       /**< 本页 LVGL 根对象；未 LOAD 或已 UNLOAD 时为 NULL */
    PageManager* _Manager; /**< 指向 App.cpp 里那个唯一的 PageManager，用来 Push/Pop */
    const char* _Name;     /**< 应用名，如 "Pages/Dialplate"（Install 第二个参数） */
    const PageOps* ops;    /**< 生命周期函数表；Install 后由 PAGE_CALL 调用 */
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

        PageStash_t Stash;          /**< Push 带过来的参数区 */
        PageState_t State;          /**< 生命周期状态 */

        /** 切页动画运行时状态 */
        struct
        {
            bool IsEnter;           /**< true = 本页是进入方；false = 离开方 */
            bool IsBusy;            /**< 本页动画是否还在播 */
            PageAnimAttr_t Attr;    /**< 本页自定义动画；Type=GLOBAL 则用全局 */
        } Anim;
    } priv;
};

#define PAGE_CALL(p, fn) do { \
    if ((p) && (p)->ops && (p)->ops->fn) (p)->ops->fn(p); \
} while (0)

void Page_SetCustomCacheEnable(PageBase* page, bool en);
void Page_SetCustomAutoCacheEnable(PageBase* page, bool en);
void Page_SetCustomLoadAnimType(PageBase* page, uint8_t animType, uint16_t time, lv_anim_path_cb_t path);
bool page_stash_pop(PageBase* page, void* ptr, uint32_t size);
bool page_push(PageBase* page, const char* name, const PageStash_t* stash);
bool page_pop(PageBase* page);
bool page_replace(PageBase* page, const char* name, const PageStash_t* stash);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ! __PAGE_BASE_H */
