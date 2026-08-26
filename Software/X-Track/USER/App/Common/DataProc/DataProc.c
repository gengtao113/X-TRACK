#include "DataProc.h"
#include "lvgl/lvgl.h"

static DataCenter center;
static int center_inited;

static void DataProc_EnsureCenter(void)
{
    if (!center_inited)
    {
        DataCenter_Init(&center, "CENTER");
        center_inited = 1;
    }
}

void* DataProc_Center(void)
{
    DataProc_EnsureCenter();
    return &center;
}

void* DataProc_MainAccount(void)
{
    DataProc_EnsureCenter();
    return &center.AccountMain;
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

#define DP_DEF(NODE_NAME, BUFFER_SIZE) DATA_PROC_INIT_DEF(NODE_NAME);
#include "DP_LIST.inc"
#undef DP_DEF

void DataProc_Init(void)
{
    DataProc_EnsureCenter();

#define DP_DEF(NODE_NAME, BUFFER_SIZE)\
    Account* act##NODE_NAME = Account_Create(#NODE_NAME, &center, BUFFER_SIZE, NULL);
#  include "DP_LIST.inc"
#undef DP_DEF

#define DP_DEF(NODE_NAME, BUFFER_SIZE)\
    _DP_##NODE_NAME##_Init(act##NODE_NAME);
#  include "DP_LIST.inc"
#undef DP_DEF
}
