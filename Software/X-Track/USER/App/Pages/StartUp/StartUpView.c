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
#include "StartUpView.h"
#include "Version.h"
#include "Resource/ResourcePool.h"
#include "Utils/lv_ext/lv_anim_timeline_wrapper.h"

#define COLOR_ORANGE    lv_color_hex(0xff931e)
#define LV_ANIM_EXEC(attr)      (lv_anim_exec_xcb_t)lv_obj_set_##attr

void StartupView_Create(StartupView* view, lv_obj_t* root)
{
    lv_obj_t* cont = lv_obj_create(root);
    lv_obj_remove_style_all(cont);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont, 110, 50);
    lv_obj_set_style_border_color(cont, COLOR_ORANGE, 0);
    lv_obj_set_style_border_side(cont, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(cont, 3, 0);
    lv_obj_set_style_border_post(cont, true, 0);
    lv_obj_center(cont);
    view->ui.cont = cont;

    lv_obj_t* label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool_GetFont("agencyb_36"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, VERSION_FIRMWARE_NAME);
    lv_obj_center(label);
    view->ui.labelLogo = label;

    view->ui.anim_timeline = lv_anim_timeline_create();

#define ANIM_DEF(start_time, obj, attr, start, end) \
     {start_time, obj, LV_ANIM_EXEC(attr), start, end, 500, lv_anim_path_ease_out, true}

    lv_anim_timeline_wrapper_t wrapper[] =
    {
        ANIM_DEF(0, view->ui.cont, width, 0, lv_obj_get_style_width(view->ui.cont, 0)),
        ANIM_DEF(500, view->ui.labelLogo, y, lv_obj_get_style_height(view->ui.cont, 0), lv_obj_get_y(view->ui.labelLogo)),
        LV_ANIM_TIMELINE_WRAPPER_END
    };

    lv_anim_timeline_add_wrapper(view->ui.anim_timeline, wrapper);
}

void StartupView_Delete(StartupView* view)
{
    if (view->ui.anim_timeline)
    {
        lv_anim_timeline_del(view->ui.anim_timeline);
        view->ui.anim_timeline = NULL;
    }
}
