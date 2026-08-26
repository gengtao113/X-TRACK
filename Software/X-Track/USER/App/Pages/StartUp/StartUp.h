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
#ifndef __STARTUP_PRESENTER_H
#define __STARTUP_PRESENTER_H

#include "StartUpView.h"
#include "StartUpModel.h"
#include "Utils/PageManager/PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  开机页 Presenter
  * @note   约 2 秒后 Replace 到表盘。base 必须是第一项。
  */
typedef struct
{
    PageBase base;
    StartupView view;
    StartupModel model;
} StartupPage;

/**
  * @brief  创建开机页（静态单例）
  * @note   工厂 CreatePage("Startup") 调这个，不再 new。
  */
PageBase* Startup_Create(void);

#ifdef __cplusplus
}
#endif

#endif
