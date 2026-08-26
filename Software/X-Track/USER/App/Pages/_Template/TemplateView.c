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
#include "TemplateView.h"

/**
  * @brief  在页面 root 下创建两个 Label
  * @note   标题文案在 Presenter on_load 里写成 _Name；
  *         tick 文案在 Update 里刷新。此处先放空字符串。
  */
void TemplateView_Create(TemplateView* view, lv_obj_t* root)
{
    lv_obj_t* label = lv_label_create(root);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text(label, "");
    view->ui.labelTitle = label;

    label = lv_label_create(root);
    lv_label_set_text(label, "");
    lv_obj_center(label);
    view->ui.labelTick = label;
}
