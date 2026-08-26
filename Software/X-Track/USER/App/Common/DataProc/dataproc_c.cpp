#include "dataproc_c.h"
#include "DataProc.h"
#include "lvgl/lvgl.h"

void* DataProc_Center(void)
{
    return DataProc::Center();
}

uint32_t DataProc_GetTick(void)
{
    return lv_tick_get();
}

uint32_t DataProc_GetTickElaps(uint32_t prevTick)
{
    return lv_tick_elaps(prevTick);
}

const char* DataProc_MakeTimeString(uint64_t ms, char* buf, uint16_t len)
{
    uint64_t ss = ms / 1000;
    uint64_t mm = ss / 60;
    uint32_t hh = (uint32_t)(mm / 60);

    lv_snprintf(
        buf, len,
        "%d:%02d:%02d",
        hh,
        (uint32_t)(mm % 60),
        (uint32_t)(ss % 60)
    );

    return buf;
}
