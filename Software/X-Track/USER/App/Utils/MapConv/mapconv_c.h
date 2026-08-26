#ifndef __MAPCONV_C_H
#define __MAPCONV_C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
class MapConv;
#else
typedef struct MapConv MapConv;
#endif

#ifdef __cplusplus
extern "C" {
#endif

MapConv* MapConv_Create(void);
void     MapConv_Destroy(MapConv* m);
void     MapConv_SetLevel(MapConv* m, int level);
int16_t  MapConv_GetLevel(MapConv* m);
void     MapConv_ConvertMapCoordinate(MapConv* m, double longitude, double latitude, int32_t* mapX, int32_t* mapY);
void     MapConv_SetDirPath(const char* path);
void     MapConv_SetExtName(const char* name);
void     MapConv_SetCoordTransformEnable(bool en);
void     MapConv_SetLevelRange(int16_t min, int16_t max);
int16_t  MapConv_GetLevelMin(void);
int16_t  MapConv_GetLevelMax(void);

#ifdef __cplusplus
}
#endif

#endif
