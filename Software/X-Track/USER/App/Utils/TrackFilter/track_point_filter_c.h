#ifndef __TRACK_POINT_FILTER_C_H
#define __TRACK_POINT_FILTER_C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
class TrackPointFilter;
#else
typedef struct TrackPointFilter TrackPointFilter;
#endif

#ifdef __cplusplus
extern "C" {
#endif

TrackPointFilter* TrackPointFilter_Create(void);
void              TrackPointFilter_Destroy(TrackPointFilter* f);
void              TrackPointFilter_Reset(TrackPointFilter* f);
bool              TrackPointFilter_PushPoint(TrackPointFilter* f, double x, double y);
void              TrackPointFilter_SetOffsetThreshold(TrackPointFilter* f, double offset);
void              TrackPointFilter_GetCounts(TrackPointFilter* f, uint32_t* sum, uint32_t* output);

#ifdef __cplusplus
}
#endif

#endif
