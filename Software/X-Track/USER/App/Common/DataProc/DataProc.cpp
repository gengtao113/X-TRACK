#include "DataProc.h"
#include "dataproc_c.h"

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

DataCenter* DataProc::Center()
{
    DataProc_EnsureCenter();
    return &center;
}

uint32_t DataProc::GetTick()
{
    return DataProc_GetTick();
}

uint32_t DataProc::GetTickElaps(uint32_t prevTick)
{
    return DataProc_GetTickElaps(prevTick);
}

const char* DataProc::MakeTimeString(uint64_t ms, char* buf, uint16_t len)
{
    return DataProc_MakeTimeString(ms, buf, len);
}

#define DP_DEF(NODE_NAME, BUFFER_SIZE) DATA_PROC_INIT_DEF(NODE_NAME);
#include "DP_LIST.inc"
#undef DP_DEF

void DataProc_Init()
{
    DataProc_EnsureCenter();

#define DP_DEF(NODE_NAME, BUFFER_SIZE)\
    Account* act##NODE_NAME = Account_Create(#NODE_NAME, &center, BUFFER_SIZE, nullptr);
#  include "DP_LIST.inc"
#undef DP_DEF

#define DP_DEF(NODE_NAME, BUFFER_SIZE)\
    _DP_##NODE_NAME##_Init(act##NODE_NAME);
#  include "DP_LIST.inc"
#undef DP_DEF
}
