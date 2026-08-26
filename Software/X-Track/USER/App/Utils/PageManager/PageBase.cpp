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
#include "PageBase.h"
#include "PageManager.h"
#include "PM_Log.h"
#include <string.h>

/**
  * @brief  手动指定本页是否缓存（切走后 root 还在不在）
  * @param  page  页面
  * @param  en    true = 保留 root；false = 切走 UNLOAD
  * @note   会先关掉自动缓存。应在 onCustomAttrConfig 里调用。
  *         LOAD 结束时：若手动模式，IsCached = ReqEnableCache。
  * @retval None
  */
void Page_SetCustomCacheEnable(PageBase* page, bool en)
{
    PM_LOG_INFO("Page(%s) %s = %d", page->_Name, __func__, en);
    Page_SetCustomAutoCacheEnable(page, false);
    page->priv.ReqEnableCache = en;
}

/**
  * @brief  是否让调度器自动管理缓存
  * @param  page  页面
  * @param  en    true = 自动（默认进页后 IsCached=true）；false = 手动
  * @note   内部存的是反标志 ReqDisableAutoCache = !en。
  * @retval None
  */
void Page_SetCustomAutoCacheEnable(PageBase* page, bool en)
{
    PM_LOG_INFO("Page(%s) %s = %d", page->_Name, __func__, en);
    page->priv.ReqDisableAutoCache = !en;
}

/**
  * @brief  设置本页切页动画（覆盖全局 SetGlobalLoadAnimType）
  * @param  page      页面
  * @param  animType  PageManager::LoadAnim_t；LOAD_ANIM_GLOBAL 则仍跟全局
  * @param  time      时长 ms
  * @param  path      缓动曲线
  * @retval None
  */
void Page_SetCustomLoadAnimType(
    PageBase* page,
    uint8_t animType,
    uint16_t time,
    lv_anim_path_cb_t path
)
{
    page->priv.Anim.Attr.Type = animType;
    page->priv.Anim.Attr.Time = time;
    page->priv.Anim.Attr.Path = path;
}

/**
  * @brief  从本页 stash 拷出数据并释放内部缓冲
  * @param  page  页面
  * @param  ptr   调用方缓冲区（如 Param_t *）
  * @param  size  必须等于 Push 时 stash 的 size
  * @retval true 成功并已 lv_mem_free；false 失败
  */
bool page_stash_pop(PageBase* page, void* ptr, uint32_t size)
{
    if (page->priv.Stash.ptr == nullptr)
    {
        PM_LOG_WARN("No Stash found");
        return false;
    }

    if (page->priv.Stash.size != size)
    {
        PM_LOG_WARN(
            "Stash[0x%p](%d) does not match the size(%d)",
            page->priv.Stash.ptr,
            page->priv.Stash.size,
            size
        );
        return false;
    }

    memcpy(ptr, page->priv.Stash.ptr, page->priv.Stash.size);
    lv_mem_free(page->priv.Stash.ptr);
    page->priv.Stash.ptr = nullptr;
    return true;
}

/**
  * @brief  C 页面压栈进入新页
  * @note   转调 page->_Manager->Push。stash 可为空。
  */
bool page_push(PageBase* page, const char* name, const PageStash_t* stash)
{
    if (page == nullptr || page->_Manager == nullptr)
    {
        return false;
    }
    return page->_Manager->Push(name, stash);
}

/**
  * @brief  C 页面弹出当前页
  * @note   转调 page->_Manager->Pop。模板页短按/LEAVE 用这个。
  */
bool page_pop(PageBase* page)
{
    if (page == nullptr || page->_Manager == nullptr)
    {
        return false;
    }
    return page->_Manager->Pop();
}

/**
  * @brief  C 页面替换当前页（开机页进表盘）
  * @note   转调 page->_Manager->Replace。stash 可为空。
  */
bool page_replace(PageBase* page, const char* name, const PageStash_t* stash)
{
    if (page == nullptr || page->_Manager == nullptr)
    {
        return false;
    }
    return page->_Manager->Replace(name, stash);
}
