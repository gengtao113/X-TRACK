#ifndef __POINT_CONTAINER_C_H
#define __POINT_CONTAINER_C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* PointContainer_Create(void);
void  PointContainer_Destroy(void* pc);
void  PointContainer_PushPoint(void* pc, int32_t x, int32_t y);

#ifdef __cplusplus
}
#endif

#endif
