#include "point_container_c.h"
#include "PointContainer.h"

void* PointContainer_Create(void)
{
    return new PointContainer();
}

void PointContainer_Destroy(void* pc)
{
    delete (PointContainer*)pc;
}

void PointContainer_PushPoint(void* pc, int32_t x, int32_t y)
{
    ((PointContainer*)pc)->PushPoint(x, y);
}
