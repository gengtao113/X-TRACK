#include "StartUpModel.h"
#include "Common/DataProc/DataProc.h"
#include "Common/HAL/HAL.h"

void StartupModel_Init(StartupModel* m)
{
    Account* account = Account_Create("StartupModel", DataProc::Center(), 0, m);
    Account_Subscribe(account, "MusicPlayer");
    Account_Subscribe(account, "StatusBar");
    m->account = account;
}

void StartupModel_Deinit(StartupModel* m)
{
    if (m->account)
    {
        Account_Destroy((Account*)m->account);
        m->account = nullptr;
    }
}

void StartupModel_PlayMusic(StartupModel* m, const char* music)
{
    DataProc::MusicPlayer_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.music = music;
    Account_Notify((Account*)m->account, "MusicPlayer", &info, sizeof(info));
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
    Account_Notify((Account*)m->account, "StatusBar", &info, sizeof(info));
}
