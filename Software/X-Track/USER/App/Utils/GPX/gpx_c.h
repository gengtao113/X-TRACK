#ifndef __GPX_C_H
#define __GPX_C_H

#include <stdint.h>

#ifdef __cplusplus
class GPX;
#else
typedef struct GPX GPX;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GPX_TRKPT
#define GPX_TRKPT "trkpt"
#endif

GPX* GPX_Create(void);
void GPX_Destroy(GPX* g);
void GPX_SetMetaName(GPX* g, const char* name);
void GPX_SetMetaDesc(GPX* g, const char* desc);
void GPX_SetName(GPX* g, const char* name);
void GPX_SetDesc(GPX* g, const char* desc);
void GPX_SetEle(GPX* g, const char* ele);
void GPX_SetTime(GPX* g, const char* time);
void GPX_GetOpen(GPX* g, char* buf, uint32_t len);
void GPX_GetClose(GPX* g, char* buf, uint32_t len);
void GPX_GetMetaData(GPX* g, char* buf, uint32_t len);
void GPX_GetTrakOpen(GPX* g, char* buf, uint32_t len);
void GPX_GetTrakClose(GPX* g, char* buf, uint32_t len);
void GPX_GetTrakSegOpen(GPX* g, char* buf, uint32_t len);
void GPX_GetTrakSegClose(GPX* g, char* buf, uint32_t len);
void GPX_GetInfo(GPX* g, char* buf, uint32_t len);
void GPX_GetPt(GPX* g, const char* typ, const char* lon, const char* lat, char* buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
