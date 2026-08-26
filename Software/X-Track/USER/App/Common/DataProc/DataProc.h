#ifndef __DATA_PROC_H
#define __DATA_PROC_H

#include <string.h>
#include "../HAL/HAL_Def.h"
#include "DataProc_Def.h"

#ifdef __cplusplus
#include "Utils/DataCenter/DataCenter.h"
#else
#include "Utils/DataCenter/account_c.h"
#endif

#ifdef __cplusplus
#define DATA_PROC_INIT_DEF(name) extern "C" void _DP_##name##_Init(Account* account)
#else
#define DATA_PROC_INIT_DEF(name) void _DP_##name##_Init(Account* account)
#endif

#define DATA_PROC_INIT_STRUCT(sct) memset(&(sct), 0, sizeof(sct))

#ifdef __cplusplus
extern "C" {
#endif
void DataProc_Init(void);
#ifdef __cplusplus
}

namespace DataProc
{

DataCenter* Center();
uint32_t    GetTick();
uint32_t    GetTickElaps(uint32_t prevTick);
const char* MakeTimeString(uint64_t ms, char* buf, uint16_t len);

}
#endif

#endif
