#ifndef __DIALPLATE_MODEL_H
#define __DIALPLATE_MODEL_H

#include "Utils/DataCenter/account_c.h"
#include "Common/HAL/HAL_Def.h"
#include "Common/DataProc/DataProc_Def.h"
#include <stdbool.h>

#ifdef __cplusplus
typedef HAL::SportStatus_Info_t SportStatus_Info_t;
typedef DataProc::StatusBar_Style_t StatusBar_Style_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DIALPLATE_REC_START = 0,
    DIALPLATE_REC_PAUSE,
    DIALPLATE_REC_CONTINUE,
    DIALPLATE_REC_STOP,
    DIALPLATE_REC_READY_STOP
} DialplateRecCmd_t;

typedef struct
{
    SportStatus_Info_t sportStatusInfo;
    Account* account;
} DialplateModel;

void  DialplateModel_Init(DialplateModel* m);
void  DialplateModel_Deinit(DialplateModel* m);
bool  DialplateModel_GetGPSReady(DialplateModel* m);
float DialplateModel_GetSpeed(const DialplateModel* m);
float DialplateModel_GetAvgSpeed(const DialplateModel* m);
void  DialplateModel_RecorderCommand(DialplateModel* m, DialplateRecCmd_t cmd);
void  DialplateModel_PlayMusic(DialplateModel* m, const char* music);
void  DialplateModel_SetStatusBarStyle(DialplateModel* m, StatusBar_Style_t style);

#ifdef __cplusplus
}
#endif

#endif
