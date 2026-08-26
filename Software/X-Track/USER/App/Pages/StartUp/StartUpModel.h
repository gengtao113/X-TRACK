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
#ifndef __STARTUP_MODEL_H
#define __STARTUP_MODEL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  开机页 Model
  * @note   实现仍是 .cpp（Account / HAL 还是 C++）。步骤 4 再换成 account_c。
  */
typedef struct
{
    void* account;  /**< Account*，只在 StartUpModel.cpp 里用 */
} StartupModel;

void StartupModel_Init(StartupModel* m);
void StartupModel_Deinit(StartupModel* m);
void StartupModel_PlayMusic(StartupModel* m, const char* music);
void StartupModel_SetEncoderEnable(bool en);
void StartupModel_SetStatusBarAppear(StartupModel* m, bool en);

#ifdef __cplusplus
}
#endif

#endif
