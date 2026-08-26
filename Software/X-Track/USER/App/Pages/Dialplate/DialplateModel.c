#include "DialplateModel.h"
#include "Common/DataProc/dataproc_c.h"
#include <string.h>

static int DialplateModel_OnEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    DialplateModel* m;

    if (event != ACCOUNT_EVENT_PUB_PUBLISH)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (strcmp(Account_GetID((Account*)from), "SportStatus") != 0
        || size != sizeof(SportStatus_Info_t))
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }

    m = (DialplateModel*)Account_GetUser(account);
    memcpy(&(m->sportStatusInfo), data, size);
    return ACCOUNT_RES_OK;
}

void DialplateModel_Init(DialplateModel* m)
{
    m->account = Account_Create("DialplateModel", DataProc_Center(), 0, m);
    Account_Subscribe(m->account, "SportStatus");
    Account_Subscribe(m->account, "Recorder");
    Account_Subscribe(m->account, "StatusBar");
    Account_Subscribe(m->account, "GPS");
    Account_Subscribe(m->account, "MusicPlayer");
    Account_SetCallback(m->account, DialplateModel_OnEvent);
}

void DialplateModel_Deinit(DialplateModel* m)
{
    if (m->account)
    {
        Account_Destroy(m->account);
        m->account = NULL;
    }
}

bool DialplateModel_GetGPSReady(DialplateModel* m)
{
    GPS_Info_t gps;
    if (Account_Pull(m->account, "GPS", &gps, sizeof(gps)) != ACCOUNT_RES_OK)
    {
        return false;
    }
    return (gps.satellites > 0);
}

float DialplateModel_GetSpeed(const DialplateModel* m)
{
    return m->sportStatusInfo.speedKph;
}

float DialplateModel_GetAvgSpeed(const DialplateModel* m)
{
    return m->sportStatusInfo.speedAvgKph;
}

void DialplateModel_RecorderCommand(DialplateModel* m, DialplateRecCmd_t cmd)
{
    StatusBar_Info_t statInfo;

    if (cmd != DIALPLATE_REC_READY_STOP)
    {
        Recorder_Info_t recInfo;
        memset(&recInfo, 0, sizeof(recInfo));
        recInfo.cmd = (Recorder_Cmd_t)cmd;
        recInfo.time = 1000;
        Account_Notify(m->account, "Recorder", &recInfo, sizeof(recInfo));
    }

    memset(&statInfo, 0, sizeof(statInfo));
    statInfo.cmd = STATUS_BAR_CMD_SET_LABEL_REC;

    switch (cmd)
    {
    case DIALPLATE_REC_START:
    case DIALPLATE_REC_CONTINUE:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "REC";
        break;
    case DIALPLATE_REC_PAUSE:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "PAUSE";
        break;
    case DIALPLATE_REC_READY_STOP:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "STOP";
        break;
    case DIALPLATE_REC_STOP:
        statInfo.param.labelRec.show = false;
        break;
    default:
        break;
    }

    Account_Notify(m->account, "StatusBar", &statInfo, sizeof(statInfo));
}

void DialplateModel_PlayMusic(DialplateModel* m, const char* music)
{
    MusicPlayer_Info_t info;
    memset(&info, 0, sizeof(info));
    info.music = music;
    Account_Notify(m->account, "MusicPlayer", &info, sizeof(info));
}

void DialplateModel_SetStatusBarStyle(DialplateModel* m, StatusBar_Style_t style)
{
    StatusBar_Info_t info;
    memset(&info, 0, sizeof(info));
    info.cmd = STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;
    Account_Notify(m->account, "StatusBar", &info, sizeof(info));
}
