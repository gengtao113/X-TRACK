#ifndef __SYSTEM_INFOS_MODEL_H
#define __SYSTEM_INFOS_MODEL_H

#include "Utils/DataCenter/account_c.h"
#include "Common/DataProc/DataProc_Def.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
typedef DataProc::StatusBar_Style_t StatusBar_Style_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    Account* account;
} SystemInfosModel;

void SystemInfosModel_Init(SystemInfosModel* m);
void SystemInfosModel_Deinit(SystemInfosModel* m);
void SystemInfosModel_GetSportInfo(SystemInfosModel* m, float* trip, char* time, uint32_t len, float* maxSpd);
void SystemInfosModel_GetGPSInfo(SystemInfosModel* m, float* lat, float* lng, float* alt, char* utc, uint32_t len, float* course, float* speed);
void SystemInfosModel_GetMAGInfo(SystemInfosModel* m, float* dir, int* x, int* y, int* z);
void SystemInfosModel_GetIMUInfo(SystemInfosModel* m, int* step, char* info, uint32_t len);
void SystemInfosModel_GetRTCInfo(SystemInfosModel* m, char* dateTime, uint32_t len);
void SystemInfosModel_GetBatteryInfo(SystemInfosModel* m, int* usage, float* voltage, char* state, uint32_t len);
void SystemInfosModel_GetStorageInfo(SystemInfosModel* m, bool* detect, const char** type, char* size, uint32_t len);
void SystemInfosModel_SetStatusBarStyle(SystemInfosModel* m, StatusBar_Style_t style);

#ifdef __cplusplus
}
#endif

#endif
