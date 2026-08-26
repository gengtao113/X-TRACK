#ifndef __DATAPROC_C_H
#define __DATAPROC_C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void*       DataProc_Center(void);
void*       DataProc_MainAccount(void);
uint32_t    DataProc_GetTick(void);
uint32_t    DataProc_GetTickElaps(uint32_t prevTick);
const char* DataProc_MakeTimeString(uint64_t ms, char* buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
