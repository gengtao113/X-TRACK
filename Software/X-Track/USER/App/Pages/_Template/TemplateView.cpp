#include "TemplateView.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Page;  ///< 本文件里 TemplateView 等于 Page::TemplateView

/**
  * @brief  在页面 root 下创建两个 Label
  * @param  root  PageManager 为本页准备的全屏容器
  * @note   只碰 LVGL。标题文案在 Presenter onViewLoad 里写成 _Name；
  *         tick 文案在 Update 里刷新。此处先放空字符串。
  * @retval None
  */
void TemplateView::Create(lv_obj_t* root)
{
    lv_obj_t* label = lv_label_create(root);   ///< 顶部标题 Label
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20); ///< 顶中对齐，下移 20px
    lv_label_set_text(label, "");               ///< 占位，稍后由 Presenter 写入页面名
    ui.labelTitle = label;                      ///< 交给 Presenter：View.ui.labelTitle

    label = lv_label_create(root);              ///< 中间 tick Label
    lv_label_set_text(label, "");               ///< 占位，稍后 Update 写成 tick = ...
    lv_obj_center(label);                       ///< 居中
    ui.labelTick = label;                       ///< 交给 Presenter：View.ui.labelTick
}
