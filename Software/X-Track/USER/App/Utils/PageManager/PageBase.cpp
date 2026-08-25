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
#include "PageBase.h"
#include "PM_Log.h"

/**
  * @brief  手动指定本页是否缓存（切走后 root 还在不在）
  * @param  en  true = 保留 root；false = 切走 UNLOAD
  * @note   会先关掉自动缓存。应在 onCustomAttrConfig 里调用。
  *         LOAD 结束时：若手动模式，IsCached = ReqEnableCache。
  * @retval None
  */
void PageBase::SetCustomCacheEnable(bool en)
{
    PM_LOG_INFO("Page(%s) %s = %d", _Name, __func__, en);
    SetCustomAutoCacheEnable(false);   ///< 关掉自动，改由本页 ReqEnableCache 说了算
    priv.ReqEnableCache = en;          ///< true：Startup/LiveMap 关缓存；Template 开缓存
}

/**
  * @brief  是否让调度器自动管理缓存
  * @param  en  true = 自动（默认进页后 IsCached=true）；false = 手动
  * @note   内部存的是反标志 ReqDisableAutoCache = !en。
  * @retval None
  */
void PageBase::SetCustomAutoCacheEnable(bool en)
{
    PM_LOG_INFO("Page(%s) %s = %d", _Name, __func__, en);
    priv.ReqDisableAutoCache = !en;    ///< true 表示「不要自动缓存」
}

/**
  * @brief  设置本页切页动画（覆盖全局 SetGlobalLoadAnimType）
  * @param  animType  PageManager::LoadAnim_t；LOAD_ANIM_GLOBAL 则仍跟全局
  * @param  time      时长 ms
  * @param  path      缓动曲线
  * @retval None
  */
void PageBase::SetCustomLoadAnimType(
    uint8_t animType,
    uint16_t time,
    lv_anim_path_cb_t path
)
{
    priv.Anim.Attr.Type = animType;    ///< 写入 priv，SwitchTo 时拷到 _AnimState.Current
    priv.Anim.Attr.Time = time;
    priv.Anim.Attr.Path = path;
}

/**
  * @brief  从本页 stash 拷出数据并释放内部缓冲
  * @param  ptr   调用方缓冲区（如 Param_t *）
  * @param  size  必须等于 Push 时 stash 的 size
  * @note   宏 PAGE_STASH_POP(var) 展开为本调用。无 stash 或长度不对返回 false，var 保持原值。
  * @retval true 成功并已 lv_mem_free；false 失败
  */
bool PageBase::StashPop(void* ptr, uint32_t size)
{
    if (priv.Stash.ptr == nullptr)     ///< Push 时没带参数，或已经 Pop 过
    {
        PM_LOG_WARN("No Stash found");
        return false;
    }

    if (priv.Stash.size != size)       ///< 与 PAGE_STASH_MAKE 的 sizeof 不一致
    {
        PM_LOG_WARN(
            "Stash[0x%p](%d) does not match the size(%d)",
            priv.Stash.ptr,
            priv.Stash.size,
            size
        );
        return false;
    }

    memcpy(ptr, priv.Stash.ptr, priv.Stash.size);  ///< 拷到调用方栈上的变量
    lv_mem_free(priv.Stash.ptr);                   ///< SwitchTo 里 lv_mem_alloc 的那块
    priv.Stash.ptr = nullptr;
    return true;
}

