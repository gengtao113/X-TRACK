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
#ifndef __TEMPLATE_VIEW_H
#define __TEMPLATE_VIEW_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  模板页 View：标题 + tick 两个 Label
  * @note   只碰 LVGL。文案由 Presenter 写入。
  */
typedef struct
{
    struct
    {
        lv_obj_t* labelTitle;  /**< 顶部标题 */
        lv_obj_t* labelTick;   /**< 中间 tick */
    } ui;
} TemplateView;

/**
  * @brief  在页面 root 下创建两个 Label
  * @param  view  本页 View
  * @param  root  PageManager 为本页准备的全屏容器
  */
void TemplateView_Create(TemplateView* view, lv_obj_t* root);

#ifdef __cplusplus
}
#endif

#endif
