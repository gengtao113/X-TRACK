#include "gpx_c.h"
#include "GPX.h"
#include <string.h>

static void GPX_Copy(const String& s, char* buf, uint32_t len)
{
    if (buf == nullptr || len == 0)
    {
        return;
    }
    strncpy(buf, s.c_str(), len - 1);
    buf[len - 1] = '\0';
}

GPX* GPX_Create(void)
{
    return new GPX();
}

void GPX_Destroy(GPX* g)
{
    delete g;
}

void GPX_SetMetaName(GPX* g, const char* name)
{
    g->setMetaName(name);
}

void GPX_SetMetaDesc(GPX* g, const char* desc)
{
    g->setMetaDesc(desc);
}

void GPX_SetName(GPX* g, const char* name)
{
    g->setName(name);
}

void GPX_SetDesc(GPX* g, const char* desc)
{
    g->setDesc(desc);
}

void GPX_SetEle(GPX* g, const char* ele)
{
    g->setEle(ele);
}

void GPX_SetTime(GPX* g, const char* time)
{
    g->setTime(time);
}

void GPX_GetOpen(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getOpen(), buf, len);
}

void GPX_GetClose(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getClose(), buf, len);
}

void GPX_GetMetaData(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getMetaData(), buf, len);
}

void GPX_GetTrakOpen(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getTrakOpen(), buf, len);
}

void GPX_GetTrakClose(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getTrakClose(), buf, len);
}

void GPX_GetTrakSegOpen(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getTrakSegOpen(), buf, len);
}

void GPX_GetTrakSegClose(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getTrakSegClose(), buf, len);
}

void GPX_GetInfo(GPX* g, char* buf, uint32_t len)
{
    GPX_Copy(g->getInfo(), buf, len);
}

void GPX_GetPt(GPX* g, const char* typ, const char* lon, const char* lat, char* buf, uint32_t len)
{
    GPX_Copy(g->getPt(typ, lon, lat), buf, len);
}
