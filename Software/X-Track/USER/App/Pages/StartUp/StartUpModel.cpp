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
#include "StartUpModel.h"
#include "Common/DataProc/DataProc.h"
#include "Common/HAL/HAL.h"

void StartupModel_Init(StartupModel* m)
{
    Account* account = new Account("StartupModel", DataProc::Center(), 0, m);
    account->Subscribe("MusicPlayer");
    account->Subscribe("StatusBar");
    m->account = account;
}

void StartupModel_Deinit(StartupModel* m)
{
    if (m->account)
    {
        delete (Account*)m->account;
        m->account = nullptr;
    }
}

void StartupModel_PlayMusic(StartupModel* m, const char* music)
{
    DataProc::MusicPlayer_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.music = music;
    ((Account*)m->account)->Notify("MusicPlayer", &info, sizeof(info));
}

void StartupModel_SetEncoderEnable(bool en)
{
    HAL::Encoder_SetEnable(en);
}

void StartupModel_SetStatusBarAppear(StartupModel* m, bool en)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.cmd = DataProc::STATUS_BAR_CMD_APPEAR;
    info.param.appear = en;
    ((Account*)m->account)->Notify("StatusBar", &info, sizeof(info));
}
