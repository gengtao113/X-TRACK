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
#include "PageManager.h"
#include "PM_Log.h"

/**
  * @brief  按动画类型填一张参数表（起止坐标/透明度 + 拖拽方向 + setter/getter）
  * @param  anim  PageManager::LoadAnim_t
  * @param  attr  输出
  * @note   push = 进入新页时用；pop = 返回时用（方向相反）。
  *         enter = 新出现的那一页；exit = 正在离开的那一页。
  *         OVER：exit.end 仍为 0，旧页原地不动；MOVE：旧页被推到 ±hor/±ver。
  *         数值是 x 或 y（像素），FADE 则是透明度。0 表示停在屏幕内原位。
  * @retval true 成功；未知类型 false
  */
bool PageManager::GetLoadAnimAttr(uint8_t anim, LoadAnimAttr_t* attr)
{
    lv_coord_t hor = LV_HOR_RES;  ///< 屏宽，水平滑入/滑出的距离
    lv_coord_t ver = LV_VER_RES;  ///< 屏高，垂直滑入/滑出的距离

    switch (anim)
    {
    case LOAD_ANIM_OVER_LEFT:     /* 新页从右侧进来盖住旧页；可水平拖返回 */
        attr->dragDir = ROOT_DRAG_DIR_HOR;

        attr->push.enter.start = hor;   ///< 新页从屏幕右侧外
        attr->push.enter.end = 0;       ///< 滑到 x=0
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;        ///< 旧页不动

        attr->pop.enter.start = 0;      ///< 返回时底下那页本来就在
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = hor;       ///< 当前页滑回右侧外
        break;

    case LOAD_ANIM_OVER_RIGHT:    /* 新页从左侧进来盖住 */
        attr->dragDir = ROOT_DRAG_DIR_HOR;

        attr->push.enter.start = -hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;

        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -hor;
        break;

    case LOAD_ANIM_OVER_TOP:      /* 本工程全局默认：新页从下方滑上来盖住 */
        attr->dragDir = ROOT_DRAG_DIR_VER;

        attr->push.enter.start = ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;

        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = ver;
        break;

    case LOAD_ANIM_OVER_BOTTOM:   /* 新页从上方滑下来盖住（模板页用这个） */
        attr->dragDir = ROOT_DRAG_DIR_VER;

        attr->push.enter.start = -ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;

        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -ver;
        break;

    case LOAD_ANIM_MOVE_LEFT:     /* 新页从右进，同时把旧页推向左侧 */
        attr->dragDir = ROOT_DRAG_DIR_HOR;

        attr->push.enter.start = hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = -hor;     ///< 与 OVER 的差别：旧页也动

        attr->pop.enter.start = -hor;   ///< 返回时旧页从左侧滑回
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = hor;
        break;

    case LOAD_ANIM_MOVE_RIGHT:
        attr->dragDir = ROOT_DRAG_DIR_HOR;

        attr->push.enter.start = -hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = hor;

        attr->pop.enter.start = hor;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -hor;
        break;

    case LOAD_ANIM_MOVE_TOP:
        attr->dragDir = ROOT_DRAG_DIR_VER;

        attr->push.enter.start = ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = -ver;

        attr->pop.enter.start = -ver;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = ver;
        break;

    case LOAD_ANIM_MOVE_BOTTOM:
        attr->dragDir = ROOT_DRAG_DIR_VER;

        attr->push.enter.start = -ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = ver;

        attr->pop.enter.start = ver;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -ver;
        break;

    case LOAD_ANIM_FADE_ON:       /* 改透明度，不能拖拽返回 */
        attr->dragDir = ROOT_DRAG_DIR_NONE;

        attr->push.enter.start = LV_OPA_TRANSP;  ///< 新页从全透明
        attr->push.enter.end = LV_OPA_COVER;
        attr->push.exit.start = LV_OPA_COVER;
        attr->push.exit.end = LV_OPA_COVER;      ///< 旧页保持不透明被盖住

        attr->pop.enter.start = LV_OPA_COVER;
        attr->pop.enter.end = LV_OPA_COVER;
        attr->pop.exit.start = LV_OPA_COVER;
        attr->pop.exit.end = LV_OPA_TRANSP;      ///< 当前页淡出
        break;

    case LOAD_ANIM_NONE:          /* 表盘/开机：无位移也无淡入 */
        memset(attr, 0, sizeof(LoadAnimAttr_t));
        return true;

    default:
        PM_LOG_ERROR("Load anim type error: %d", anim);
        return false;
    }

    /* 按拖拽方向决定动画改 x、改 y，还是改透明度。
     * [](...){ } 是 C++ lambda，当函数指针用；C 里就是三个 static 函数。 */
    if (attr->dragDir == ROOT_DRAG_DIR_HOR)
    {
        attr->setter = [](void* obj, int32_t v)
        {
            lv_obj_set_x((lv_obj_t*)obj, v);
        };
        attr->getter = [](void* obj)
        {
            return (int32_t)lv_obj_get_x((lv_obj_t*)obj);
        };
    }
    else if (attr->dragDir == ROOT_DRAG_DIR_VER)
    {
        attr->setter = [](void* obj, int32_t v)
        {
            lv_obj_set_y((lv_obj_t*)obj, v);
        };
        attr->getter = [](void* obj)
        {
            return (int32_t)lv_obj_get_y((lv_obj_t*)obj);
        };
    }
    else
    {
        attr->setter = [](void* obj, int32_t v)
        {
            lv_obj_set_style_bg_opa((lv_obj_t*)obj, (lv_opa_t)v, LV_PART_MAIN);
        };
        attr->getter = [](void* obj)
        {
            return (int32_t)lv_obj_get_style_bg_opa((lv_obj_t*)obj, LV_PART_MAIN);
        };
    }

    return true;
}
