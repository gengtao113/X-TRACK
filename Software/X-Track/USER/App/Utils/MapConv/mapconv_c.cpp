#include "mapconv_c.h"
#include "MapConv.h"

MapConv* MapConv_Create(void)
{
    return new MapConv();
}

void MapConv_Destroy(MapConv* m)
{
    delete m;
}

void MapConv_SetLevel(MapConv* m, int level)
{
    m->SetLevel(level);
}

int16_t MapConv_GetLevel(MapConv* m)
{
    return m->GetLevel();
}

void MapConv_ConvertMapCoordinate(MapConv* m, double longitude, double latitude, int32_t* mapX, int32_t* mapY)
{
    m->ConvertMapCoordinate(longitude, latitude, mapX, mapY);
}

void MapConv_SetDirPath(const char* path)
{
    MapConv::SetDirPath(path);
}

void MapConv_SetExtName(const char* name)
{
    MapConv::SetExtName(name);
}

void MapConv_SetCoordTransformEnable(bool en)
{
    MapConv::SetCoordTransformEnable(en);
}

void MapConv_SetLevelRange(int16_t min, int16_t max)
{
    MapConv::SetLevelRange(min, max);
}

int16_t MapConv_GetLevelMin(void)
{
    return MapConv::GetLevelMin();
}

int16_t MapConv_GetLevelMax(void)
{
    return MapConv::GetLevelMax();
}
