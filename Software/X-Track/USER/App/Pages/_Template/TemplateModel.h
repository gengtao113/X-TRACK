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
#ifndef __TEMPLATE_MODEL_H
#define __TEMPLATE_MODEL_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  模板页 Model：示范「可以极瘦」
  * @note   没有 DataCenter 账号，不 Subscribe。正式页面应在这里 Pull/Notify。
  */
typedef struct
{
    uint32_t TickSave;  /**< 进入页面时记下的 tick，on_load 里赋值 */
} TemplateModel;

/**
  * @brief  取当前 LVGL 毫秒 tick
  * @note   实现就是 return lv_tick_get()。正式页应改成问总线要数据。
  */
uint32_t TemplateModel_GetData(void);

#ifdef __cplusplus
}
#endif

#endif
